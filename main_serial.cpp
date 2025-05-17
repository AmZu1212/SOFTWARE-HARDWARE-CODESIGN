/**************************************************
 *                                                *
 *       main() for testing nbody serial          *
 *                                                *
 *               Written by:                      *
 *            Amir Zuabi - 212606222              *
 *             Nir Schif - 212980395              *
 *                                                *
 **************************************************/

#include "nbody_serial.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <chrono>
using namespace std;

extern const int nParticles;
extern ParticleType serialParticles[];

/**
 * @brief Writes particle data to a text file in human-readable format.
 * 
 * @param filename The name of the output file (e.g., "serial_result.txt")
 */
void SaveParticlesToFile(const string& filename)
{
    ofstream out(filename);
    for (int i = 0; i < nParticles; i++) {
        out << serialParticles[i].x << ' '
            << serialParticles[i].y << ' '
            << serialParticles[i].z << ' '
            << serialParticles[i].vx << ' '
            << serialParticles[i].vy << ' '
            << serialParticles[i].vz << '\n';
    }
    out.close();
}

/**
 * @brief Main entry point for the serial N-body simulation.
 * 
 * The number of steps must be passed as a command-line argument. This function
 * initializes the particles, performs the simulation, and saves the results to a file.
 * 
 * @param argc Number of command-line arguments.
 * @param argv Command-line arguments.
 * @return int Exit code (0 on success, 1 on failure).
 */
int main(int argc, char** argv) {
    cout << "\n---  Starting Serial Run ---\n";
    cout << "\n---  Initializing Particles ---\n";

    // Ensure user provides number of simulation steps
    if (argc < 2) {
        cerr << "❌ Error: Please provide the number of steps, e.g., ./serial.exe 10" << endl;
        return 1;
    }

    int maxSteps = std::stoi(argv[1]);

    // Initialize all particle positions and velocities
    InitParticleSerial();

    // Run simulation for the given number of steps
    for (int step = 1; step <= maxSteps; ++step) {
        cout << "\n--- Serial Step " << step << " ---\n";

        auto start = std::chrono::high_resolution_clock::now();
        MoveParticlesSerial();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        cout << "Serial step time: " << duration << " ms" << endl;
    }

    // Save final particle state to output file
    auto start = std::chrono::high_resolution_clock::now();
    SaveParticlesToFile("serial_result.txt");
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    cout << "\n === Serial saving time time: " << duration << " ms ===\n" << endl;
    cout << "Serial simulation complete. Results saved to serial_result.txt" << endl;
    return 0;
}
