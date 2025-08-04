// ===== File: validate.cpp =====
#include <iostream>
#include <fstream>
#include <cmath>
#include <chrono>
using namespace std;

// Acceptable error margin when comparing particle positions
const float EPSILON = 0.1f;

// Number of particles expected in each file (must match simulation)
const int nParticles = 16384 * 2; // adjust if needed

/**
 * @brief Compares two simulation output files line-by-line for positional accuracy.
 * 
 * Reads particle data from both files and compares the x, y, and z values.
 * If any difference exceeds EPSILON, a mismatch is reported.
 * 
 * @param file1 Path to the first result file (e.g., "serial_result.txt")
 * @param file2 Path to the second result file (e.g., "parallel_result.txt")
 * @return true if all particles match within EPSILON, false otherwise.
 */
bool CompareResults(const string& file1, const string& file2) {
    ifstream in1(file1), in2(file2);
    if (!in1.is_open() || !in2.is_open()) {
        cerr << "Error opening files." << endl;
        return false;
    }

    float x1, y1, z1, vx1, vy1, vz1;
    float x2, y2, z2, vx2, vy2, vz2;

    for (int i = 0; i < nParticles; i++) {
        in1 >> x1 >> y1 >> z1 >> vx1 >> vy1 >> vz1;
        in2 >> x2 >> y2 >> z2 >> vx2 >> vy2 >> vz2;

        if (fabs(x1 - x2) > EPSILON || fabs(y1 - y2) > EPSILON || fabs(z1 - z2) > EPSILON) {
            cout << "Mismatch at particle " << i << ": dx=" << fabs(x1 - x2)
                 << " dy=" << fabs(y1 - y2) << " dz=" << fabs(z1 - z2) << endl;
            return false;
        }
    }

    return true;
}

/**
 * @brief Main entry point for result validation between serial and parallel runs.
 * 
 * Compares the particle position output files and reports success or failure.
 * 
 * @return int Exit code (0 for success, 1 for mismatch or error).
 */
int main() {
    if (CompareResults("serial_result.txt", "parallel_result.txt")) {
        cout << "Validation successful. Outputs match within epsilon." << endl;
        return 0;
    } else {
        cout << "Validation failed. Outputs mismatch." << endl;
        return 1;
    }
}
