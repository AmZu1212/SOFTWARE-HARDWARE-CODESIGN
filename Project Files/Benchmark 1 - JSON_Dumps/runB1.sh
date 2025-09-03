#!/bin/bash

set -e  # Exit on error

# Authored by Amir Zuabi & Nir Schif & GPT5


# make sure you update the directory which your flamegraph is installed at, and the directory where the pyperformance benchmarks are located.
# usually in: /usr/local/lib/python3.10/dist-packages/pyperformance/data-files/benchmarks/bm_json_dumps/
# for python 3.X you'll need to change the number obviously.


# this script runs both versions of the benchmark, json std and json orjson.
# makes the flamegraphs for you, and outputs the perfs in the perf outputs folder.

# to see the stats in the console liek we showcased in out project document, we used this line.
# unfortunately you cannot create a Flamegraph and showcase these stats at the same time,
# so we left it out of the script. you can use this command and some editing of this script to make it appear,
# or just do it manually with your prefered version (json/orjson) and see the stats :)


# perf stat -e cycles,instructions,branches,branch-misses,cache-misses -- python3-dbg -m pyperformance run --bench json_dumps


# === Config ===
WORKDIR="$(pwd)"
ASSETDIR="$WORKDIR/Helper Assets"
FLAMEOUTDIR="$WORKDIR/FlameGraph Outputs"
PERFOUTDIR="$WORKDIR/Perf Outputs"
TIMESTAMP=$(date +"%Y%m%d-%H%M%S")
FLAMEDIR="../../../FlameGraph"
BENCH_FILE="/usr/local/lib/python3.10/dist-packages/pyperformance/data-files/benchmarks/bm_json_dumps/run_benchmark.py"

# === Create output directories if missing ===
mkdir -p "$FLAMEOUTDIR"
mkdir -p "$PERFOUTDIR"

# === Function to run one benchmark pass ===
run_benchmark_pass() {
    local SRC_SCRIPT="$1"
    local VARIANT_LABEL="$2"  # e.g., "std" or "orjson"

    echo "[*] Copying $SRC_SCRIPT → $BENCH_FILE"
    cp "$SRC_SCRIPT" "$BENCH_FILE"

    echo "[*] Running perf on 'json_dumps ($VARIANT_LABEL)'..."
    perf record -F 999 -g -o "$PERFOUTDIR/perf_${VARIANT_LABEL}.data" -- python3-dbg -m pyperformance run --bench json_dumps

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
