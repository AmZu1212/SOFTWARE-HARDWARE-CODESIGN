/**************************************************
 *                                                *
 *   main for testing nbody serial and parallel   *
 *                                                *
 *               Written by:                      *
 *            Amir Zuabi - 212606222              *
 *             Nir Schif - 212980395              *
 *                                                *
 **************************************************/


// Implementation Includes
#include "nbody_parallel.hpp"
#include "nbody_serial.hpp"

// System Includes
#include <iostream>
#include <sys/time.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <chrono>  // <-- Timer

// namespaces and signatures
using namespace std;
void TestMultipleSteps(int maxSteps);


// Global Particle Arrays
OneParticle serialParticles[nParticles];
OneParticle parallelParticles[nParticles];




// Validate that serial and parallel results match
bool ValidateParticles()
{
    // some value to ignore small floatign point errors
    const float error = 0.1f;

    // loop over all the particles
    for (int i = 0; i < nParticles; i++)
    {
        // get particle pair
        OneParticle serialP = serialParticles[i];
        OneParticle parallelP = parallelParticles[i];

        // calc deltas
        const float dx = fabs(serialP.x - parallelP.x);
        const float dy = fabs(serialP.y - parallelP.y);
        const float dz = fabs(serialP.z - parallelP.z);

        // Check Margains
        if (dx >= error || dy >= error || dz >= error)
        {
            //print error
            cout << "Mismatch at index " << i << ": dx=" << dx << " dy=" << dy << " dz=" << dz << endl;
            
            // mismatch found, bad.
            return false;
        }
    }
    
    // no mismatch found, good.
    return true;
}


int main()
{
    cout << "\nPropagating " << nParticles << " particles using serial implementation on CPU\n" << endl;

    int maxSteps = 5;
    int threadCount = thread::hardware_concurrency();
    cout << "\nThread Count: " << threadCount << endl;

    InitParticleSerial();
    for (int i = 0; i < nParticles; i++)
    {
        // Copy serial particles to global arrays for parallel to start with the same state
        global_X[i] = serialParticles[i].x;
        global_Y[i] = serialParticles[i].y;
        global_Z[i] = serialParticles[i].z;
        global_Vx[i] = serialParticles[i].vx;
        global_Vy[i] = serialParticles[i].vy;
        global_Vz[i] = serialParticles[i].vz;
    }

    // Serial steps with timing
    for (int step = 1; step <= maxSteps; ++step)
    {
        cout << "\n--- Serial Step " << step << " ---\n";

        auto start = std::chrono::high_resolution_clock::now();

        MoveParticlesSerial();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        cout << "Serial step time: " << duration << " ms" << endl;
    }

    // Parallel steps with timing
    for (int step = 1; step <= maxSteps; ++step)
    {
        cout << "\n--- Parallel Step " << step << " ---\n";

        auto start = std::chrono::high_resolution_clock::now();

        StartThreads(MoveChunk);
        StartThreads(UpdateChunkPosition);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        cout << "Parallel step time: " << duration << " ms" << endl;

        for (int i = 0; i < nParticles; i++)
        {
            parallelParticles[i].x = global_X[i];
            parallelParticles[i].y = global_Y[i];
            parallelParticles[i].z = global_Z[i];
            parallelParticles[i].vx = global_Vx[i];
            parallelParticles[i].vy = global_Vy[i];
            parallelParticles[i].vz = global_Vz[i];
        }
    }

    if (ValidateParticles())
    {
        cout << "Validation successful for step!" << endl;
    }
    else
    {
        cout << "Validation failed at step. Results mismatch!" << endl;
    }

    return 0;
}


/**
 * @brief Performs a step-by-stmentations side by side for the specified
 * number of steps, comparing their results after each step and printing mismatches if any.
 *ep comparison between Serial and Parallel N-Body implementations.
 *
 * This function initializes both the Serial and Parallel particle states to the same
 * starting configuration. It then runs both imple
 * @param maxSteps The number of simulation steps to perform and validate.
 */
void TestMultipleSteps(int maxSteps)
{
    cout << "\n--- Multi-Step Comparison (" << maxSteps << " steps) ---\n" << endl;

    // Reinitialize particles to the same starting state
    InitParticleSerial();
    for (int i = 0; i < nParticles; i++)
    {
        // Copy serial particles to global arrays for parallel to start with the same state
        global_X[i] = serialParticles[i].x;
        global_Y[i] = serialParticles[i].y;
        global_Z[i] = serialParticles[i].z;
        global_Vx[i] = serialParticles[i].vx;
        global_Vy[i] = serialParticles[i].vy;
        global_Vz[i] = serialParticles[i].vz;
    }

    for (int step = 1; step <= maxSteps; ++step)
    {
        cout << "\n--- Step " << step << " ---\n";

        // Serial step
        MoveParticlesSerial();

        // Parallel step
        StartThreads(MoveChunk);
        StartThreads(UpdateChunkPosition);

        for (int i = 0; i < nParticles; i++)
        {
            parallelParticles[i].x = global_X[i];
            parallelParticles[i].y = global_Y[i];
            parallelParticles[i].z = global_Z[i];
            parallelParticles[i].vx = global_Vx[i];
            parallelParticles[i].vy = global_Vy[i];
            parallelParticles[i].vz = global_Vz[i];
        }

        // Print first 15 particles
        /*for (int i = 0; i < 15; i++)
        {
            cout << "Particle " << i << ":"
                 << " Serial(x=" << serialParticles[i].x << ", y=" << serialParticles[i].y << ", z=" << serialParticles[i].z << ")"
                 << " | Parallel(x=" << parallelParticles[i].x << ", y=" << parallelParticles[i].y << ", z=" << parallelParticles[i].z << ")\n";
        }*/

        // Validate and report
        if (ValidateParticles())
        {
            cout << "Validation successful for step " << step << "!" << endl;
        }
        else
        {
            cout << "Validation failed at step " << step << ". Results mismatch!" << endl;
            break;  // Stop on first failure
        }
    }
}
