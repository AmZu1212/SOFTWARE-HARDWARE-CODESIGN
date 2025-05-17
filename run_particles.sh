#!/bin/bash

# Written by Amir Zuabi & Nir Schif
# This script cleans, builds, runs, and validates the N-body simulations.

# this is the simulation step count parameter. by default it is 5.
STEPS=10

# toggle flamegraph generation
GENERATE_FLAMEGRAPH=TRUE

# Flamegraph generation function
generate_flamegraph() {
    local binary=$1
    local steps=$2
    local output_name=$3

    echo ""
    echo "🔥 Running perf and generating flamegraph for $binary..."

    perf record -F 99 -g ./$binary $steps
    perf script > ${output_name}.perf
    stackcollapse-perf.pl ${output_name}.perf | awk -F';' 'NF <= 50' > ${output_name}.folded
    mkdir -p Flamegraphs
    flamegraph.pl --minwidth 1.0 --title="${output_name^} Flamegraph" ${output_name}.folded > Flamegraphs/flamegraph_${output_name}.svg

    # Clean up intermediate files
    rm -f perf.data perf.data.old ${output_name}.perf ${output_name}.folded

    echo "✅ Flamegraph generated: Flamegraphs/flamegraph_${output_name}.svg"
}


echo ""
# Step 1: Clean build artifacts
echo "🧹   Cleaning build artifacts   🧹"
make clean

echo ""
# Step 2: Delete old result files
echo "🗑️    Removing previous result files   🗑️"
rm -f serial_result.txt parallel_result.txt

echo ""
# Step 3: Build all targets
echo "🔨   Building executables   🔨"
make serial.exe parallel.exe validate.exe cache.exe
if [ $? -ne 0 ]; then
    echo "❌   Build failed. Aborting execution."
    exit 1
fi

echo ""
# Step 4: Run serial simulation
echo -e "\n⚙️    Running serial simulation   ⚙️"
if [ "$GENERATE_FLAMEGRAPH" == "TRUE" ]; then
    generate_flamegraph "serial.exe" $STEPS "serial"
else
    ./serial.exe $STEPS
fi

echo ""
# Step 4.5: Trash cache to ensure purity before parallel
echo -e "\n🧹   Trashing CPU cache before parallel run   🧹"
time ./cache.exe

# Note, cache trashing doesnt affect outcome at all due to larger data size.

echo ""
# Step 5: Run parallel simulation
echo -e "\n⚙️    Running parallel simulation   ⚙️"
if [ "$GENERATE_FLAMEGRAPH" == "TRUE" ]; then
    generate_flamegraph "parallel.exe" $STEPS "parallel"
else
    ./parallel.exe $STEPS
fi

echo ""
# Step 6: Run validation
echo -e "\n🔍   Validating results   🔍"
./validate.exe

echo ""
