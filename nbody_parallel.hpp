#include <cmath>
#include <thread>
#include <vector>
#include <immintrin.h>
using namespace std;


// Written by: Amir Zuabi - 212606222
//             Nir Schif  - 212980395

//      Note: this code runs 30x faster than the original, due to cache optimization, vectorization (AVX2) and multi-threading.

//      Note: __m256 needs to be supported to run this (AVX2)

//      Note: 256 = 32 bytes = 8 floats/ints

//      Note: This means unit stride of 8 per vector

#ifndef N_BODY_CONSTS
#define N_BODY_CONSTS
// Simulation parameters
const int nParticles = 16384 * 2;
const float dt = 0.01f;
const float softening = 1e-20f;

// Particle definition (not used here but left for consistency)
typedef struct
{
    float x, y, z, vx, vy, vz;
} OneParticle;
#endif 

// Note: notice how we moved from a particle array to coords arrays, in order to improve cache locality.
// Global arrays for particle positions and velocities
float global_X[nParticles];
float global_Y[nParticles];
float global_Z[nParticles];
float global_Vx[nParticles];
float global_Vy[nParticles];
float global_Vz[nParticles];

// Preloaded SIMD vectors for constants
__m256 zeroVector = _mm256_set1_ps(0.0f);
__m256 oneVector = _mm256_set1_ps(1.0f);
__m256 dtVector = _mm256_set1_ps(dt);
__m256 softVector = _mm256_set1_ps(softening);

// Threading configuration
const unsigned int NUM_THREADS = thread::hardware_concurrency(); // Detect number of CPU cores, originally designed for 12 cores, 24 threads.
const unsigned int CHUNK_SIZE = nParticles / NUM_THREADS;        // Divide work evenly across threads

// Initializes particle positions and velocities in parallel
void InitChunk(unsigned int start, unsigned int end)
{
    for (unsigned int i = start; i < end; ++i)
    {
        global_X[i] = (float)(i % 15);
        global_Y[i] = (float)((i * i) % 15);
        global_Z[i] = (float)((i * i * 3) % 15);
        global_Vx[i] = 0.0f;
        global_Vy[i] = 0.0f;
        global_Vz[i] = 0.0f;
    }
}

// Calculates forces on particles and updates velocities in parallel
void MoveChunk(unsigned int start, unsigned int end)
{
    for (unsigned int i = start; i < end; ++i)
    {
        // Accumulated force components for particle i
        float Fx = 0, Fy = 0, Fz = 0;
        __m256 FxVector = zeroVector;
        __m256 FyVector = zeroVector;
        __m256 FzVector = zeroVector;

        // Load current particle position as SIMD vector
        __m256 PixVector = _mm256_set1_ps(global_X[i]);
        __m256 PiyVector = _mm256_set1_ps(global_Y[i]);
        __m256 PizVector = _mm256_set1_ps(global_Z[i]);

        // Iterate over all particles in vectorized blocks of 8
        for (unsigned int j = 0; j < nParticles; j += 8)
        {
            // Load positions of 8 particles
            __m256 PjxVector = _mm256_loadu_ps(&global_X[j]);
            __m256 PjyVector = _mm256_loadu_ps(&global_Y[j]);
            __m256 PjzVector = _mm256_loadu_ps(&global_Z[j]);

            // Compute displacement vectors
            __m256 dx = _mm256_sub_ps(PjxVector, PixVector);
            __m256 dy = _mm256_sub_ps(PjyVector, PiyVector);
            __m256 dz = _mm256_sub_ps(PjzVector, PizVector);

            // Compute squared distance + softening
            __m256 dx2 = _mm256_mul_ps(dx, dx);
            __m256 dy2 = _mm256_mul_ps(dy, dy);
            __m256 dz2 = _mm256_mul_ps(dz, dz);
            
            
            // denominator calcs (this is dumb but refactor later)
            __m256 temp1 = _mm256_add_ps(dx2, softVector);
            __m256 temp2 = _mm256_add_ps(dy2, dz2);
            __m256 temp3 = _mm256_add_ps(temp1, temp2);
            __m256 distSqr = _mm256_sqrt_ps(temp3);

            // Compute 1 / distance and its cube
            __m256 invDist = _mm256_div_ps(oneVector, distSqr);
            __m256 invDist3 = _mm256_mul_ps(invDist, _mm256_mul_ps(invDist, invDist));

            // Accumulate forces
            FxVector = _mm256_add_ps(FxVector, _mm256_mul_ps(dx, invDist3));
            FyVector = _mm256_add_ps(FyVector, _mm256_mul_ps(dy, invDist3));
            FzVector = _mm256_add_ps(FzVector, _mm256_mul_ps(dz, invDist3));
        }

        // Unpack SIMD accumulations to scalar
        float *TempArray = (float *)&FxVector;
        Fx = TempArray[0] + TempArray[1] + TempArray[2] + TempArray[3] +
             TempArray[4] + TempArray[5] + TempArray[6] + TempArray[7];

        TempArray = (float *)&FyVector;
        Fy = TempArray[0] + TempArray[1] + TempArray[2] + TempArray[3] +
             TempArray[4] + TempArray[5] + TempArray[6] + TempArray[7];

        TempArray = (float *)&FzVector;
        Fz = TempArray[0] + TempArray[1] + TempArray[2] + TempArray[3] +
             TempArray[4] + TempArray[5] + TempArray[6] + TempArray[7];

        // Update particle velocity using computed forces
        global_Vx[i] += dt * Fx;
        global_Vy[i] += dt * Fy;
        global_Vz[i] += dt * Fz;
    }
}

// Updates particle positions based on their velocities in parallel
void UpdateChunkPosition(unsigned int start, unsigned int end)
{
    for (unsigned int i = start; i < end; ++i)
    {
        global_X[i] += global_Vx[i] * dt;
        global_Y[i] += global_Vy[i] * dt;
        global_Z[i] += global_Vz[i] * dt;
    }
}


// Generic parallel execution helper for any chunked operation
void StartThreads(void (*func)(unsigned int, unsigned int))
{
    vector<thread> threads;
    for (unsigned int t = 0; t < NUM_THREADS; ++t)
    {
        unsigned int start = t * CHUNK_SIZE;
        unsigned int end = (t == NUM_THREADS - 1) ? nParticles : start + CHUNK_SIZE;
        threads.emplace_back(func, start, end);
    }

    for (auto &th : threads)
    {
        th.join();
    }
}
