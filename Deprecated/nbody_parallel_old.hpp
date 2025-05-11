#include <cmath>
#include <tbb/tbb.h>        // for TBB
#include <immintrin.h>      // for AVX
using namespace std;



#define TBBGRAIN 1024       // 4 (SIZE OF FLOAT) * 6 (SIZE OF PARTICLE) * 1024(TBB GRAIN) =~ 24KB

#ifndef N_BODY_CONSTS
#define N_BODY_CONSTS
    const int nParticles = 16384 * 2;
    const float dt = 0.01f;
    const float softening = 1e-20f;

    typedef struct
    {
        float x, y, z, vx, vy, vz;
    } OneParticle;
#endif

// Currently designed for AVX-256 machines, can be used with __m128 aswell, need to figure out this syntax.
float global_X[nParticles];
float global_Y[nParticles];
float global_Z[nParticles];
float global_Vx[nParticles];
float global_Vy[nParticles];
float global_Vz[nParticles];



// ========== SUPPORT VECTORS =========
__m256 zeroVector = _mm256_set1_ps(0.0f);
__m256 oneVector = _mm256_set1_ps(1.0f);
__m256 dtVector = _mm256_set1_ps(dt);
__m256 softVector = _mm256_set1_ps(softening);


// this gets 30x speed up!!!!
// boring, no optimizations here.
void get_particle_parallel(int i, OneParticle *p){
    p->x = global_X[i];
    p->y = global_Y[i];
    p->z = global_Z[i];
    p->vx = global_Vx[i];
    p->vy = global_Vy[i];
    p->vz = global_Vz[i];
}


void init_particles_parallel()
{
    tbb::parallel_for( // (range, lambda)
    tbb::blocked_range<unsigned int>(0, nParticles, TBBGRAIN),
    [](tbb::blocked_range<unsigned int> &range)
    {
        for (unsigned int i = range.begin(); i < range.end(); i++)
        {
            global_X[i]  = (float)(i % 15);
            global_Y[i]  = (float)(((i * i) % 15));
            global_Z[i]  = (float)((i * i * 3) % 15);
            global_Vx[i] = 0.0f;
            global_Vy[i] = 0.0f;
            global_Vz[i] = 0.0f;
        }
    });
}



void move_particles_parallel()
{
    tbb::parallel_for(tbb::blocked_range<unsigned int>(0, nParticles, TBBGRAIN),
    [](const tbb::blocked_range<unsigned int> &range)
    {
        for (unsigned int i = range.begin(); i < range.end(); ++i)
        {

            // Components of the gravity force on particle i
            float Fx = 0, Fy = 0, Fz = 0;
            __m256 FxVector = zeroVector;
            __m256 FyVector = zeroVector;
            __m256 FzVector = zeroVector;

            // creation of x, y, z vectors for calculations. because in the j iterations they stay the same.
            __m256 PixVector = _mm256_set1_ps(global_X[i]);
            __m256 PiyVector = _mm256_set1_ps(global_Y[i]);
            __m256 PizVector = _mm256_set1_ps(global_Z[i]);

            // i will vectorize in jumps of j+8's.
            for (unsigned int j = 0; j < nParticles; j += 8)
            {
                // creation of more x, y, z vectors for dx/y/z.
                __m256 PjxVector = _mm256_loadu_ps(&global_X[j]);
                __m256 PjyVector = _mm256_loadu_ps(&global_Y[j]);
                __m256 PjzVector = _mm256_loadu_ps(&global_Z[j]);

                // Newton's law of universal gravity
                __m256 dxVector = _mm256_sub_ps(PjxVector, PixVector);
                __m256 dyVector = _mm256_sub_ps(PjyVector, PiyVector);
                __m256 dzVector = _mm256_sub_ps(PjzVector, PizVector);

                // a TON of calcs for R1 ^ 3
                __m256 dxPow2 = _mm256_mul_ps(dxVector, dxVector);
                __m256 dyPow2 = _mm256_mul_ps(dyVector, dyVector);
                __m256 dzPow2 = _mm256_mul_ps(dzVector, dzVector);
                
                // denominator calcs
                __m256 temp1 = _mm256_add_ps(dxPow2, softVector);
                __m256 temp2 = _mm256_add_ps(dyPow2, dzPow2);
                __m256 temp3 = _mm256_add_ps(temp1, temp2);
                __m256 denominatorVector = _mm256_sqrt_ps(temp3);

                // finally RR1
                __m256 drVector = _mm256_div_ps(oneVector, denominatorVector);

                // now ot the power of 3
                __m256 drPower3Vector = _mm256_mul_ps(drVector, _mm256_mul_ps(drVector, drVector));

                // Calculate the net force
                FxVector = _mm256_add_ps(FxVector, _mm256_mul_ps(dxVector, drPower3Vector));
                FyVector = _mm256_add_ps(FyVector, _mm256_mul_ps(dyVector, drPower3Vector));
                FzVector = _mm256_add_ps(FzVector, _mm256_mul_ps(dzVector, drPower3Vector));
            }

            // step out of vectorization for velocity calculations. need to unvectorize Fx/y/z.
            // this works 100%.
            float *TempArray = (float*) &FxVector;
            Fx = TempArray[0] + TempArray[1] + TempArray[2] + TempArray[3]
                + TempArray[4] + TempArray[5] + TempArray[6] + TempArray[7];
            
            TempArray = (float*) &FyVector;
            Fy = TempArray[0] + TempArray[1] + TempArray[2] + TempArray[3]
                + TempArray[4] + TempArray[5] + TempArray[6] + TempArray[7];

            TempArray = (float*) &FzVector;
            Fz = TempArray[0] + TempArray[1] + TempArray[2] + TempArray[3]
                + TempArray[4] + TempArray[5] + TempArray[6] + TempArray[7];

            // Accelerate particles in response to the gravitational force
            global_Vx[i] += dt * Fx;
            global_Vy[i] += dt * Fy;
            global_Vz[i] += dt * Fz;
        }
    });

    // Move particles according to their velocities
    tbb::parallel_for(tbb::blocked_range<unsigned int>(0, nParticles, TBBGRAIN),
    [](const tbb::blocked_range<unsigned int> &range)
    {
        for (unsigned int i = range.begin(); i < range.end(); ++i)
        {
            global_X[i] += global_Vx[i] * dt;
            global_Y[i] += global_Vy[i] * dt;
            global_Z[i] += global_Vz[i] * dt;
        }
    });
}
