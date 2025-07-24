#pragma once
#include "Chunk.h"
#include "VulkanApp.h"
#include <vector>

void greedyMesh(const Chunk& chunk, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
