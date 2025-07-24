#include "Mesher.h"

// Very simplified greedy meshing: merge along X axis for top faces only
void greedyMesh(const Chunk& chunk, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    const int S = Chunk::SIZE;
    for (int z = 0; z < S; ++z) {
        for (int x = 0; x < S; ) {
            int start = x;
            while (x < S && chunk.get(x, S-1, z).type != 0) {
                ++x;
            }
            if (start != x) {
                // Create a quad covering [start,x) at top layer
                Vertex v0{{(float)start, (float)S, (float)z},   {0,1,0}, {0,0}};
                Vertex v1{{(float)x,     (float)S, (float)z},   {0,1,0}, {1,0}};
                Vertex v2{{(float)x,     (float)S, (float)z+1}, {0,1,0}, {1,1}};
                Vertex v3{{(float)start, (float)S, (float)z+1}, {0,1,0}, {0,1}};
                uint32_t base = vertices.size();
                vertices.push_back(v0);
                vertices.push_back(v1);
                vertices.push_back(v2);
                vertices.push_back(v3);
                indices.push_back(base);
                indices.push_back(base+1);
                indices.push_back(base+2);
                indices.push_back(base);
                indices.push_back(base+2);
                indices.push_back(base+3);
            }
            ++x;
        }
    }
}
