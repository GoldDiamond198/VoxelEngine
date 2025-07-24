#include "Mesher.h"

// Very simplified greedy meshing: merge along X axis for the top surface of the
// generated terrain. The terrain generator currently fills blocks from y=0 up to
// y=SIZE/2-1, so we create a single top face at y=SIZE/2.
void greedyMesh(const Chunk& chunk, std::vector<Vertex>& vertices,
                std::vector<uint32_t>& indices) {
    const int S = Chunk::SIZE;
    const int top = S / 2 - 1;              // highest filled block
    const int yFace = top + 1;              // face sits above the highest block
    for (int z = 0; z < S; ++z) {
        for (int x = 0; x < S;) {
            int start = x;
            while (x < S && chunk.get(x, top, z).type != 0 &&
                   (top + 1 >= S || chunk.get(x, top + 1, z).type == 0)) {
                ++x;
            }
            if (start != x) {
                // Create a quad covering [start,x) at height yFace
                Vertex v0{{(float)start, (float)yFace, (float)z},   {0, 1, 0}, {0, 0}};
                Vertex v1{{(float)x,     (float)yFace, (float)z},   {0, 1, 0}, {1, 0}};
                Vertex v2{{(float)x,     (float)yFace, (float)z + 1}, {0, 1, 0}, {1, 1}};
                Vertex v3{{(float)start, (float)yFace, (float)z + 1}, {0, 1, 0}, {0, 1}};
                uint32_t base = static_cast<uint32_t>(vertices.size());
                vertices.push_back(v0);
                vertices.push_back(v1);
                vertices.push_back(v2);
                vertices.push_back(v3);
                indices.push_back(base);
                indices.push_back(base + 1);
                indices.push_back(base + 2);
                indices.push_back(base);
                indices.push_back(base + 2);
                indices.push_back(base + 3);
            }
            ++x;
        }
    }
}
