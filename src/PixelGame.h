#pragma once
#include "VulkanApp.h"
#include "ThreadPool.h"
#include "Chunk.h"
#include "Mesher.h"
#include <vector>
#include <thread>

class PixelGame {
public:
    PixelGame();
    ~PixelGame();
    void run();
private:
    VulkanApp app;
    ThreadPool pool;
    std::vector<Chunk> chunks;
    void loadWorld(std::vector<Vertex>& vertices,
                   std::vector<uint32_t>& indices);
};
