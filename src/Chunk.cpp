#include "Chunk.h"

Chunk::Chunk() {
    voxels.fill({0});
}

void Chunk::generateTestData() {
    for (int z = 0; z < SIZE; ++z) {
        for (int y = 0; y < SIZE; ++y) {
            for (int x = 0; x < SIZE; ++x) {
                if (y < SIZE / 2) {
                    voxels[index(x, y, z)].type = 1; // ground block
                }
            }
        }
    }
}

Voxel Chunk::get(int x, int y, int z) const {
    return voxels[index(x, y, z)];
}

int Chunk::index(int x, int y, int z) const {
    return x + y * SIZE + z * SIZE * SIZE;
}
