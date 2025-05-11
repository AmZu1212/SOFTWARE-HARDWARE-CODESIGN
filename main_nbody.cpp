#include "nbody_parallel.hpp"
#include "nbody_serial.hpp"
#include <iostream>
#include <sys/time.h>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

/*************************************************
 *
 *   main for testing nbody serial and parallel
 *
 *               Written by:
 *            Amir Zuabi - 212606222
 *             Nir Schif - 212980395
 *
 **************************************************/

OneParticle serialParticles[nParticles];
OneParticle parallelParticles[nParticles];

// Validate that serial and parallel results match
bool ValidateParticles()
{
    bool ret = true;
    const float epsilon = 0.1f;
    for (int i = 0; i < nParticles; i++)
    {
        OneParticle p = serialParticles[i];
        OneParticle p1 = parallelParticles[i];
        bool isEqual = fabs(p.x - p1.x) >= epsilon || fabs(p.y - p1.y) >= epsilon || fabs(p.z - p1.z) >= epsilon;
        if (isEqual)
        {
            cout << "Mismatch at index " << i << ": dx=" << p.x - p1.x << " dy=" << p.y - p1.y << " dz=" << p.z - p1.z << endl;
            ret = false;
            return ret;
        }
    }
    return ret;
}

// Configuration for benchmarking
const int nSteps = 10;
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
}

int main()
{
    cout << "\nPropagating " << nParticles << " particles using serial implementation on CPU\n"
         << endl;
    InitParticleSerial();
    RunSimulation(0);

    // Copy serial result for validation
    for (int i = 0; i < nParticles; i++)
    {
        GetParticleSerial(i, &serialParticles[i]);
    }

    cout << "\n\nPropagating " << nParticles << " particles using parallel threads on CPU\n"
         << endl;
    

    //StartThreads(InitChunk);
    RunSimulation(1);

    // Copy parallel result for validation
    for (int i = 0; i < nParticles; i++)
    {
        OneParticle p;
        p.x = global_X[i];
        p.y = global_Y[i];
        p.z = global_Z[i];
        p.vx = global_Vx[i];
        p.vy = global_Vy[i];
        p.vz = global_Vz[i];
        parallelParticles[i] = p;
    }

    if (ValidateParticles())
    {
        cout << "Validation successful!" << endl;
        cout << "Speedup: " << final_performance[0] / final_performance[1] << "x" << endl;
    }
    else
    {
        cout << "Validation failed. Results mismatch!" << endl;
    }

    return 0;
}
