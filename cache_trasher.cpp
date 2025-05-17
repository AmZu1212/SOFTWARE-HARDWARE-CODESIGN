
// This file fills cache in order to keep the two tests seperate
#include <iostream>
using namespace std;

/**
 * @brief Allocates and accesses a large memory buffer to evict CPU caches.
 * 
 * This is useful for ensuring cache state is reset between performance-critical
 * benchmarking runs to maintain result purity.
 */
void TrashCPUCache() {
    const size_t cacheTrashSize = 512 * 1024 * 1024; // 512 MB
    char* buffer = new char[cacheTrashSize];

    // Fill buffer to bring it into cache
    for (size_t i = 0; i < cacheTrashSize; i += 64) {
        buffer[i] = i % 256;
    }

    // Read back to force cache pollution
    volatile int sum = 0;
    for (size_t i = 0; i < cacheTrashSize; i += 64) {
        sum += buffer[i];
    }

    // Prevent optimization
    if (sum == 123456789) {
        cout << "Magic value hit!" << endl;
    }

    delete[] buffer;
}

int main() {
    cout << "Trashing CPU cache..." << endl;
    TrashCPUCache();
    cout << "Cache trashed successfully." << endl;
    return 0;
}
