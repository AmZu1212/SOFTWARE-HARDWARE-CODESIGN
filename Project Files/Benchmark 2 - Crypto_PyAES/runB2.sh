#!/bin/bash

set -e  # Exit on error

# Authored by Amir Zuabi & Nir Schif

# This script runs the crypto_pyaes benchmark,
# generates a flamegraph from it,
# and outputs both flamegraph SVG and perf files to organized folders.

# === Config ===
WORKDIR="$(pwd)"
BENCH_NAME="crypto_pyaes"
FLAMEOUTDIR="$WORKDIR/FlameGraph Outputs"
PERFOUTDIR="$WORKDIR/Perf Outputs"
TIMESTAMP=$(date +"%Y%m%d-%H%M%S")
FLAMEDIR="../../../FlameGraph"

# === Create output directories if missing ===
mkdir -p "$FLAMEOUTDIR"
mkdir -p "$PERFOUTDIR"

# === Run perf and generate flamegraph ===
echo "[*] Running perf on '$BENCH_NAME'..."
perf record -F 999 -g -o "$PERFOUTDIR/perf_${BENCH_NAME}.data" -- python3-dbg -m pyperformance run --bench "$BENCH_NAME"

echo "[*] Generating out.perf..."
perf script -i "$PERFOUTDIR/perf_${BENCH_NAME}.data" > "$PERFOUTDIR/out_${BENCH_NAME}.perf"

echo "[*] Moving to Flamegraph directory..."
pushd "$FLAMEDIR" > /dev/null

echo "[*] Collapsing stacks..."
./stackcollapse-perf.pl "$PERFOUTDIR/out_${BENCH_NAME}.perf" > "$PERFOUTDIR/out_${BENCH_NAME}.folded"

SVG_NAME="${BENCH_NAME}_flame_$TIMESTAMP.svg"
echo "[*] Generating flamegraph → $SVG_NAME"
./flamegraph.pl "$PERFOUTDIR/out_${BENCH_NAME}.folded" > "$FLAMEOUTDIR/$SVG_NAME"

echo "[✔] Flamegraph saved: $FLAMEOUTDIR/$SVG_NAME"

popd > /dev/null
