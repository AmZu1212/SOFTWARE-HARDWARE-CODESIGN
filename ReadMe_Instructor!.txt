Hello!

To test the code, make sure that you:

git clone https://github.com/brendangregg/Flamegraph.git flamegraph_tools
export PATH="$PATH:$PWD/flamegraph_tools"


to ensure the script works properly. 


Also dont forget to:

chmod +x run_particles.sh

And then:

./run_particles.sh


This will:

-Build the project
-Run the serial and parallel simulations
-Trash the cache between runs
-Optionally generate flamegraphs (if enabled, by default it is on)
-Validate correctness
-Output flamegraphs to the Flamegraphs/ folder as .svg's

hopefully all runs well :)
