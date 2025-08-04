/**************************************************
 *                                                *
 *        main() for testing nbody parallel       *
 *                                                *
 *               Written by:                      *
 *            Amir Zuabi - 212606222              *
 *             Nir Schif - 212980395              *
 *                                                *
 **************************************************/

#include "nbody_parallel.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <thread>
#include <chrono>
using namespace std;

extern const int nParticles;
extern float global_X[], global_Y[], global_Z[], global_Vx[], global_Vy[], global_Vz[];

/**
 * @brief Writes the final state of particles to a text file.
 * 
 * @param filename Name of the output file (e.g., "parallel_result.txt")
 */
void SaveParticlesToFile(const string& filename)
{
    ofstream out(filename);
    for (int i = 0; i < nParticles; i++) {
        out << global_X[i] << ' '
            << global_Y[i] << ' '
            << global_Z[i] << ' '
            << global_Vx[i] << ' '
            << global_Vy[i] << ' '
            << global_Vz[i] << '\n';
    }
    out.close();
}

/**
 * @brief Main entry point for the parallel N-body simulation.
 * 
 * The number of simulation steps must be provided as a command-line argument.
 * This function initializes particle states, performs the parallel simulation,
 * and saves the final result to an output file.
 * 
 * @param argc Number of command-line arguments.
 * @param argv Command-line arguments.
 * @return int Exit code (0 on success, 1 on failure).
 */
int main(int argc, char** argv) {
    cout << "\n---  Starting Parallel Run ---\n";
    cout << "\n---  Initializing Particles ---\n";

    // Ensure the user provides a step count
    if (argc < 2) {
        cerr << "❌ Error: Please provide the number of steps, e.g., ./parallel.exe 10" << endl;
        return 1;
    }

    int maxSteps = std::stoi(argv[1]);

    // Initialize particle positions and velocities in parallel
    InitChunk(0, nParticles);

    // Perform simulation steps in parallel
    for (int step = 1; step <= maxSteps; ++step) {
        cout << "\n--- Parallel Step " << step << " ---\n";
        auto start = std::chrono::high_resolution_clock::now();
        StartThreads(MoveChunk);
        StartThreads(UpdateChunkPosition);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        cout << "Parallel step time: " << duration << " ms" << endl;
    }

    // Save final simulation state to file
    auto start = std::chrono::high_resolution_clock::now();
    SaveParticlesToFile("parallel_result.txt");
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    cout << "\n === Parallel saving time time: " << duration << " ms ===\n" << endl;
    cout << "Parallel simulation complete. Results saved to parallel_result.txt" << endl;
    return 0;
}
