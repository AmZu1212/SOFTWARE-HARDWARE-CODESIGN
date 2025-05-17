#include <cmath>
using namespace std;

// Written by: Amir Zuabi - 212606222
//             Nir Schif  - 212980395


// Note: This is a serial implementation of a massivlely parallel problem.
//       It uses Newton's grav. law (check pdf) in order to calc grav. forces between serialParticles.
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

// This is a AoS - Array of Structs
struct ParticleType
{
    float x, y, z;
    float vx, vy, vz;
    float trash1, trash2;
};

ParticleType serialParticles[nParticles];


// Given an index, and a pointer, fills pointer p with particle[i]'s credentials
void GetParticleSerial(int i, OneParticle *p)
{
    p->x = serialParticles[i].x;
    p->y = serialParticles[i].y;
    p->z = serialParticles[i].z;
    p->vx = serialParticles[i].vx;
    p->vy = serialParticles[i].vy;
    p->vz = serialParticles[i].vz;
}

// Initializes the serialParticles[] array. Position is given by index.
void InitParticleSerial()
{
    for (unsigned int i = 0; i < nParticles; i++)
    {
        serialParticles[i].x = (float)(i % 15);
        serialParticles[i].y = (float)((i * i) % 15);
        serialParticles[i].z = (float)((i * i * 3) % 15);
        serialParticles[i].vx = 0;
        serialParticles[i].vy = 0;
        serialParticles[i].vz = 0;
    }
}


// Calcs serialParticles[] movement by dt time.
void MoveParticlesSerial()
{
    // Choose 1 particle to calculate force superpositions
    for (int i = 0; i < nParticles; i++)
    {
        // Components of the force exerted on particle i
        float Fx = 0, Fy = 0, Fz = 0;

        // Loop over serialParticles that exert forces
        for (int j = 0; j < nParticles; j++)
        {
            // Avoid singularity and interaction with self
            const float softening = 1e-20f;

            // Newton's law of universal gravity
            const float dx = serialParticles[j].x - serialParticles[i].x;
            const float dy = serialParticles[j].y - serialParticles[i].y;
            const float dz = serialParticles[j].z - serialParticles[i].z;

            const float dr = 1.0f / sqrt(dx * dx + dy * dy + dz * dz + softening);
            const float drPower3 = dr * dr * dr;
            // Calculate the net force
            Fx += dx * drPower3;
            Fy += dy * drPower3;
            Fz += dz * drPower3;
        }

        // Accelerate serialParticles in response to the gravitational force
        serialParticles[i].vx += dt * Fx;
        serialParticles[i].vy += dt * Fy;
        serialParticles[i].vz += dt * Fz;
    }

    // Move serialParticles according to their velocities (V = a * dt, a = F (m=1))
    // O(N) work, so using a serial loop 
    for (int i = 0; i < nParticles; i++)
    {
        serialParticles[i].x += serialParticles[i].vx * dt;
        serialParticles[i].y += serialParticles[i].vy * dt;
        serialParticles[i].z += serialParticles[i].vz * dt;
    }
}
