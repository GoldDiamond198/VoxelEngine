#pragma once
#include <array>
#include "BlockRegistry.h"

struct Voxel {
    BlockRegistry::BlockID type = 0; // 0 == air
};

class Chunk {
public:
    static const int SIZE = 16;
    Chunk();
    void generateTestData();
    Voxel get(int x, int y, int z) const;
private:
    std::array<Voxel, SIZE*SIZE*SIZE> voxels;
    int index(int x, int y, int z) const;
};
