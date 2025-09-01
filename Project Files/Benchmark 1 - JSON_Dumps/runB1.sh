#!/bin/bash

set -e  # Exit on error

# please note, that running through this script will slow down the overall results, 
# but the 6x speedup stays the same. (in some cases, instability is introduced due to the script)

# also dont forget that you need to manual change the USE_ORJSON flag in the run_benchmark.py.
# (we tried to automate this for yo uguys, but pyperformance didnt allow us to use flags. so 
# each run you'll need to toggle True/False for the improved library in the python file.)


# === Config ===
WORKDIR="$(pwd)"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
SVG_OUT="bm_json_dumps_flame_$TIMESTAMP.svg"
FLAMEDIR="../../../FlameGraph"

# === Set benchmark path manually ===
BENCH_TARGET_DIR="/usr/local/lib/python3.10/dist-packages/pyperformance/data-files/benchmarks/bm_json_dumps"
TARGET_PY="$BENCH_TARGET_DIR/run_benchmark.py"
SOURCE_PY="$WORKDIR/run_benchmark.py"

# === Overwrite the Python file ===
if [ -f "$SOURCE_PY" ]; then
    echo "[*] Copying custom run_benchmark.py to: $TARGET_PY"
    cp "$SOURCE_PY" "$TARGET_PY"
else
    echo "[✘] Source run_benchmark.py not found at: $SOURCE_PY"
    exit 1
fi

# === Run perf ===
echo "[*] Running perf on benchmark 'bm_json_dumps'..."
perf record -F 999 -g -- python3-dbg -m pyperformance run --bench json_dumps


echo "[*] Generating out.perf..."
perf script > out.perf

echo "[*] Moving to Flamegraph directory..."
pushd "$FLAMEDIR" > /dev/null

echo "[*] Collapsing stacks..."
./stackcollapse-perf.pl "$WORKDIR/out.perf" > "$WORKDIR/out.folded"

echo "[*] Generating flamegraph..."
./flamegraph.pl "$WORKDIR/out.folded" > "$WORKDIR/$SVG_OUT"

echo "[✔] Done! Flamegraph saved as: $WORKDIR/$SVG_OUT"

popd > /dev/null
