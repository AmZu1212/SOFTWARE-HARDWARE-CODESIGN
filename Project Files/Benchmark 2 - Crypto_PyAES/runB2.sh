#!/bin/bash
set -euo pipefail

# Authored by Amir Zuabi & Nir Schif & GPT

WORKDIR="$(pwd)"
ASSETDIR="$WORKDIR/Helper Assets"
FLAMEOUTDIR="$WORKDIR/FlameGraph Outputs"
PERFOUTDIR="$WORKDIR/Perf Outputs"
TIMESTAMP=$(date +"%Y%m%d-%H%M%S")
FLAMEDIR="../../../FlameGraph"
BENCH_NAME="crypto_pyaes"
PYTHON_BIN="${PYTHON_BIN:-python3-dbg}"

# === User config ===
# Toggle: when true → run perf stat (prints to terminal) instead of flamegraphs
SHOW_STATS=${SHOW_STATS:-false}
# perf stat events (comma-separated, no spaces)
PERF_EVENTS=${PERF_EVENTS:-cycles,instructions,branches,branch-misses,cache-misses,context-switches,cpu-migrations,task-clock}

# === STATIC pyperformance paths ===
PY_PERF_ROOT="/usr/local/lib/python3.10/dist-packages/pyperformance"
BENCH_DIR="$PY_PERF_ROOT/data-files/benchmarks/bm_crypto_pyaes"
BENCH_FILE="$BENCH_DIR/run_benchmark.py"
REQ_FILE="$BENCH_DIR/requirements.txt"

mkdir -p "$FLAMEOUTDIR" "$PERFOUTDIR"

# --- Backups so we leave the install clean on exit ---
BACKUP_BENCH="${BENCH_FILE}.backup.$TIMESTAMP"
BACKUP_REQ="${REQ_FILE}.backup.$TIMESTAMP"
cp "$BENCH_FILE" "$BACKUP_BENCH"
cp "$REQ_FILE" "$BACKUP_REQ"

restore_original() {
  echo "[*] Restoring original benchmark + requirements..."
  cp "$BACKUP_BENCH" "$BENCH_FILE" || true
  cp "$BACKUP_REQ"  "$REQ_FILE"    || true
}
trap restore_original EXIT

# --- Helper: run one pass ---
run_benchmark_pass() {
  local SRC_SCRIPT="$1"     # helper asset python
  local VARIANT="$2"        # orig | new

  [[ -f "$SRC_SCRIPT" ]] || { echo "[!] Missing $SRC_SCRIPT"; exit 1; }

  echo "[*] Copy $SRC_SCRIPT -> $BENCH_FILE"
  cp "$SRC_SCRIPT" "$BENCH_FILE"

  if [[ "$VARIANT" == "new" ]]; then
    echo "[*] Override requirements -> cryptography enabled"
    cp "$ASSETDIR/updated_requirements.txt" "$REQ_FILE"
  else
    echo "[*] Restore original requirements (pyaes only)"
    cp "$BACKUP_REQ" "$REQ_FILE"
  fi

  if [[ "$SHOW_STATS" == "true" ]]; then
    echo "[*] perf stat ($VARIANT)..."
    perf stat -e "$PERF_EVENTS" -- "$PYTHON_BIN" -m pyperformance run --bench "$BENCH_NAME"
    echo "[✔] perf stat completed for '$VARIANT'"
    return
  fi

  echo "[*] perf record ($VARIANT)..."
  perf record -F 999 -g \
    -o "$PERFOUTDIR/perf_${BENCH_NAME}_${VARIANT}.data" \
    -- "$PYTHON_BIN" -m pyperformance run --bench "$BENCH_NAME"

  echo "[*] perf script -> out_${BENCH_NAME}_${VARIANT}.perf"
  perf script -i "$PERFOUTDIR/perf_${BENCH_NAME}_${VARIANT}.data" \
    > "$PERFOUTDIR/out_${BENCH_NAME}_${VARIANT}.perf"

  echo "[*] FlameGraph ($VARIANT)"
  pushd "$FLAMEDIR" >/dev/null

  # Correct path (no braces bug)
  if [[ ! -f "$PERFOUTDIR/out_${BENCH_NAME}_${VARIANT}.perf" ]]; then
    echo "[!] Missing perf script output: $PERFOUTDIR/out_${BENCH_NAME}_${VARIANT}.perf"
    popd >/dev/null
    exit 1
  fi

  ./stackcollapse-perf.pl "$PERFOUTDIR/out_${BENCH_NAME}_${VARIANT}.perf" \
    > "$PERFOUTDIR/out_${BENCH_NAME}_${VARIANT}.folded"

  if [[ "$VARIANT" == "orig" ]]; then
    SVG_NAME="Original_PyAES_${TIMESTAMP}.svg"
  else
    SVG_NAME="New_PyAES_${TIMESTAMP}.svg"
  fi

  ./flamegraph.pl "$PERFOUTDIR/out_${BENCH_NAME}_${VARIANT}.folded" \
  > "$FLAMEOUTDIR/$SVG_NAME"


  popd >/dev/null
  echo "[✔] Flamegraph saved: $FLAMEOUTDIR/$SVG_NAME"
}

# --- Run both variants ---
run_benchmark_pass "$ASSETDIR/original_crypto_pyaes.py" "orig"
run_benchmark_pass "$ASSETDIR/new_crypto_pyaes.py"      "new"

echo "[✔] Done. Perf data in '$PERFOUTDIR', SVGs in '$FLAMEOUTDIR' (unless SHOW_STATS=true)."
