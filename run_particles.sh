#!/bin/bash

# Written by Amir Zuabi & Nir Schif
# This script cleans, builds, runs, and validates the N-body simulations.

# this is the simulation step count parameter. by default it is 5.
STEPS=2


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
./serial.exe $STEPS


echo ""
# Step 4.5: Trash cache to ensure purity before parallel
echo -e "\n🧹   Trashing CPU cache before parallel run   🧹"
time ./cache.exe

# Note, cache trashing doesnt affect outcome at all due to larger data size.

echo ""
# Step 5: Run parallel simulation
echo -e "\n⚙️    Running parallel simulation   ⚙️"
./parallel.exe $STEPS


echo ""
# Step 6: Run validation
echo -e "\n🔍   Validating results   🔍"
./validate.exe


echo ""


# perf & flame graph section


