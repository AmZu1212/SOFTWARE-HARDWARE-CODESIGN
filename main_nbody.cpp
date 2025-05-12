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

// performance metrics, improve this later.
// Configuration for benchmarking, make sure nsteps - skipsteps is an ODD number.
/*const int nSteps = 10;
const int skipSteps = 3;
double final_performance[2] = {1, 1};

// Timing utility
typedef struct timeval tval;
double GetElapsed(tval start, tval end)
{
    return (double)(end.tv_sec - start.tv_sec) * 1000.0 + (double)(end.tv_usec - start.tv_usec) / 1000.0;
}

// Runs a simulation and reports performance stats
void RunSimulation(int mode)
{ // mode 0: serial, mode 1: parallel
    double rate = 0;
    double total_time = 0;
    vector<double> times;

    cout << "Step\tTime(ms)\tInteract/s" << endl;

    for (int step = 1; step <= nSteps; step++)
    {
        tval start, end;
        gettimeofday(&start, NULL);

        if (mode == 0)
        {
            MoveParticlesSerial();
        }
        else
        {
            StartThreads(InitChunk);
            StartThreads(MoveChunk);
            StartThreads(UpdateChunkPosition);
        }

        gettimeofday(&end, NULL);

        const double interactions = static_cast<double>(nParticles) * (nParticles - 1);
        double time_ms = GetElapsed(start, end);

        if (step > skipSteps)
        {
            rate += (interactions / time_ms);
            total_time += time_ms;
            times.push_back(time_ms);
        }

        cout.precision(4);
        cout << step << "\t" << time_ms << "\t\t" << 1000 * interactions / time_ms << (step <= skipSteps ? "  *" : "") << endl;
    }

    sort(times.begin(), times.end());
    double median = times[times.size() / 2];
    double average = total_time / (nSteps - skipSteps);

    cout << "\n"
         << (mode == 0 ? "Serial" : "Parallel") << " performance summary:"
         << " Average(ms): " << average << " Median(ms): " << median << " Rate: " << rate << endl;
    cout << "-----------------------------------------------------" << endl;
    cout << "* - Warm-up, not included in average\n"
         << endl;

    final_performance[mode] = median;
}*/

int main()
{
    cout << "\nPropagating " << nParticles << " particles using serial implementation on CPU\n"
         << endl;
    TestMultipleSteps(10);

    return 0;
}


/**
 * @brief Performs a step-by-step comparison between Serial and Parallel N-Body implementations.
 *
 * This function initializes both the Serial and Parallel particle states to the same
 * starting configuration. It then runs both implementations side by side for the specified
 * number of steps, comparing their results after each step and printing mismatches if any.
 *
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
        for (int i = 0; i < nParticles; i++)
        {
            GetParticleSerial(i, &serialParticles[i]);
        }

        // Parallel step: InitChunk only on step 1
        if (step == 1)
        {
            StartThreads(InitChunk);
        }

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



