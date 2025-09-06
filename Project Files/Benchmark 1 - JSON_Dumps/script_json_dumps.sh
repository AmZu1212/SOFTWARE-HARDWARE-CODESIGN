#!/bin/bash
set -euo pipefail

# Authored by Amir Zuabi & Nir Schif & GPT5

# This script runs both versions of the json_dumps benchmark (stdlib json vs orjson),
# and either:
#   - Generates flamegraphs (default), or
#   - Runs perf stat and saves clean results (when SHOW_STATS=true).
#
# Output:
#   Flamegraphs → FlameGraph Outputs/
#   Perf data   → Perf Outputs/
#   Clean stats → Results/  ("Normal Results.txt" for std, "Optimized Results.txt" for orjson)

# === Config ===
WORKDIR="$(pwd)"
ASSETDIR="$WORKDIR/Helper Assets"
FLAMEOUTDIR="$WORKDIR/FlameGraph Outputs"
PERFOUTDIR="$WORKDIR/Perf Outputs"
RESULTSOUTDIR="$WORKDIR/Results"
TIMESTAMP=$(date +"%Y%m%d-%H%M%S")
FLAMEDIR="../../../FlameGraph"
BENCH_FILE="/usr/local/lib/python3.10/dist-packages/pyperformance/data-files/benchmarks/bm_json_dumps/run_benchmark.py"
JSON_REQ_FILE="$(dirname "$BENCH_FILE")/requirements.txt"
PYTHON_BIN="${PYTHON_BIN:-python3-dbg}"

# Toggle: when true → run perf stat (clean results) instead of flamegraphs
SHOW_STATS=${SHOW_STATS:-false}
# perf stat events (comma-separated, no spaces)
PERF_EVENTS=${PERF_EVENTS:-cycles,instructions,branches,branch-misses,cache-misses,context-switches,cpu-migrations,task-clock}

# === Create output directories if missing ===
mkdir -p "$FLAMEOUTDIR" "$PERFOUTDIR" "$RESULTSOUTDIR"

# --- Backups so we leave the install clean on exit ---
BACKUP_BENCH="${BENCH_FILE}.backup.$TIMESTAMP"
BACKUP_REQ="${JSON_REQ_FILE}.backup.$TIMESTAMP"

# Some pyperformance builds ship without requirements.txt; handle that.
if [[ -f "$BENCH_FILE" ]]; then
  cp "$BENCH_FILE" "$BACKUP_BENCH"
else
  echo "[!] Benchmark file not found at: $BENCH_FILE"
  exit 1
fi

if [[ -f "$JSON_REQ_FILE" ]]; then
  cp "$JSON_REQ_FILE" "$BACKUP_REQ"
else
  # create an empty backup to restore to "no extra requirements"
  touch "$BACKUP_REQ"
fi

restore_original() {
  echo "[*] Restoring original json_dumps benchmark + requirements..."
  cp "$BACKUP_BENCH" "$BENCH_FILE" || true
  if [[ -s "$BACKUP_REQ" ]]; then
    cp "$BACKUP_REQ"  "$JSON_REQ_FILE" || true
  else
    # original had no requirements file
    rm -f "$JSON_REQ_FILE" || true
  fi
}
trap restore_original EXIT

# === Function to run one benchmark pass ===
run_benchmark_pass() {
    local SRC_SCRIPT="$1"         # path to variant source (json.py / orjson.py)
    local VARIANT_LABEL="$2"      # "std" or "orjson"

    if [[ ! -f "$SRC_SCRIPT" ]]; then
        echo "[!] Missing variant source: $SRC_SCRIPT"
        exit 1
    fi

    echo "[*] Copying $SRC_SCRIPT → $BENCH_FILE"
    cp "$SRC_SCRIPT" "$BENCH_FILE"

    # For the orjson variant, force pyperformance to install orjson in its venv
    if [[ "$VARIANT_LABEL" == "orjson" ]]; then
        echo "[*] Setting requirements for orjson variant"
        printf "orjson>=3.9.0\n" > "$JSON_REQ_FILE"
    else
        echo "[*] Restoring requirements for std variant"
        if [[ -s "$BACKUP_REQ" ]]; then
          cp "$BACKUP_REQ" "$JSON_REQ_FILE"
        else
          rm -f "$JSON_REQ_FILE" || true
        fi
    fi

    if [[ "$SHOW_STATS" == "true" ]]; then
        # Stats mode: single perf stat run; capture the mean line + full perf summary.
        local OUTFILE
        if [[ "$VARIANT_LABEL" == "std" ]]; then
            OUTFILE="$RESULTSOUTDIR/Normal Results.txt"
        else
            OUTFILE="$RESULTSOUTDIR/Optimized Results.txt"
        fi

        echo "[*] perf stat (json_dumps - $VARIANT_LABEL) → $OUTFILE"
        perf stat -e "$PERF_EVENTS" \
          -- "$PYTHON_BIN" -m pyperformance run --bench json_dumps \
          2>&1 \
          | sed -r 's/\x1B\[[0-9;]*[A-Za-z]//g' \
          | awk '
              /^[[:space:]]*json_dumps: Mean \+\- std dev:/ { mean=$0; next }
              /^ *Performance counter stats/ {
                  if (!printed_mean && mean != "") { print mean "\n"; printed_mean=1 }
                  inperf=1; print; next
              }
              inperf { print; next }
              END {
                  if (!printed_mean && mean != "") { print mean }
              }
            ' | tee "$OUTFILE"

        echo "[✔] Saved clean stats: $OUTFILE"
        return
    fi

    # Flamegraph mode
    echo "[*] Running perf on 'json_dumps ($VARIANT_LABEL)'..."
    perf record -F 999 -g -o "$PERFOUTDIR/perf_${VARIANT_LABEL}.data" -- \
      "$PYTHON_BIN" -m pyperformance run --bench json_dumps

    echo "[*] Generating out.perf..."
    perf script -i "$PERFOUTDIR/perf_${VARIANT_LABEL}.data" > "$PERFOUTDIR/out_${VARIANT_LABEL}.perf"

    echo "[*] Moving to Flamegraph directory..."
    pushd "$FLAMEDIR" > /dev/null

    echo "[*] Collapsing stacks..."
    ./stackcollapse-perf.pl "$PERFOUTDIR/out_${VARIANT_LABEL}.perf" > "$PERFOUTDIR/out_${VARIANT_LABEL}.folded"

    local SVG_NAME="json_dumps_${VARIANT_LABEL}_$TIMESTAMP.svg"
    echo "[*] Generating flamegraph → $SVG_NAME"
    ./flamegraph.pl "$PERFOUTDIR/out_${VARIANT_LABEL}.folded" > "$FLAMEOUTDIR/$SVG_NAME"

    echo "[✔] Flamegraph saved: $FLAMEOUTDIR/$SVG_NAME"
    popd > /dev/null
}

# === Run both versions ===
run_benchmark_pass "$ASSETDIR/json.py" "std"
run_benchmark_pass "$ASSETDIR/orjson.py" "orjson"

if [[ "$SHOW_STATS" == "true" ]]; then
  echo "[✔] Done. Clean results saved in '$RESULTSOUTDIR'."
else
  echo "[✔] Done. Perf data in '$PERFOUTDIR', SVGs in '$FLAMEOUTDIR'."
fi
