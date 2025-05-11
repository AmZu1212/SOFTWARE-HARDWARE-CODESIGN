#include <cmath>
using namespace std;

// Written by: Amir Zuabi - 212606222
//             Nir Schif  - 212980395


// Note: This is a serial implementation of a massivlely parallel problem.
//       It uses Newton's grav. law (check pdf) in order to calc grav. forces between particles.
#ifndef N_BODY_CONSTS
#define N_BODY_CONSTS
// Simulation parameters
const int nParticles = 16384 * 2;
const float dt = 0.01f;
const float softening = 1e-20f;

typedef struct
{
    float x, y, z, vx, vy, vz;
} OneParticle;
#endif
struct ParticleType
{
    float x, y, z;
    float vx, vy, vz;
    float trash1, trash2;
};

ParticleType particles[nParticles];


// Given an index, and a pointer, fills pointer p with particle[i]'s credentials
void GetParticleSerial(int i, OneParticle *p)
{
    p->x = particles[i].x;
    p->y = particles[i].y;
    p->z = particles[i].z;
    p->vx = particles[i].vx;
    p->vy = particles[i].vy;
    p->vz = particles[i].vz;
}

// Initializes the particles[] array. Position is given by index.
void InitParticleSerial()
{
    for (unsigned int i = 0; i < nParticles; i++)
    {
        particles[i].x = (float)(i % 15);
        particles[i].y = (float)((i * i) % 15);
        particles[i].z = (float)((i * i * 3) % 15);
        particles[i].vx = 0;
        particles[i].vy = 0;
        particles[i].vz = 0;
    }
}


// Calcs particles[] movement by dt time.
void MoveParticlesSerial()
{
    // Choose 1 particle to calculate force superpositions
    for (int i = 0; i < nParticles; i++)
    {
        // Components of the force exerted on particle i
        float Fx = 0, Fy = 0, Fz = 0;

        // Loop over particles that exert forces
        for (int j = 0; j < nParticles; j++)
        {
            // Avoid singularity and interaction with self
            const float softening = 1e-20f;

            // Newton's law of universal gravity
            const float dx = particles[j].x - particles[i].x;
            const float dy = particles[j].y - particles[i].y;
            const float dz = particles[j].z - particles[i].z;

            const float dr = 1.0f / sqrt(dx * dx + dy * dy + dz * dz + softening);
            const float drPower3 = dr * dr * dr;
            // Calculate the net force
            Fx += dx * drPower3;
            Fy += dy * drPower3;
            Fz += dz * drPower3;
        }

        // Accelerate particles in response to the gravitational force
        particles[i].vx += dt * Fx;
        particles[i].vy += dt * Fy;
        particles[i].vz += dt * Fz;
    }

    // Move particles according to their velocities (V = a * dt, a = F (m=1))
    // O(N) work, so using a serial loop 
    for (int i = 0; i < nParticles; i++)
    {
        particles[i].x += particles[i].vx * dt;
        particles[i].y += particles[i].vy * dt;
        particles[i].z += particles[i].vz * dt;
    }
}
