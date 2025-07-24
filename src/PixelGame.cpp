#include "PixelGame.h"
#include <iostream>

PixelGame::PixelGame() : pool(std::thread::hardware_concurrency()) {}
PixelGame::~PixelGame() {}

void PixelGame::loadWorld(std::vector<Vertex>& vertices,
                          std::vector<uint32_t>& indices) {
    chunks.emplace_back();
    chunks.back().generateTestData();
    greedyMesh(chunks.back(), vertices, indices);
    std::cout << "Generated mesh with " << vertices.size() << " vertices\n";
}

void PixelGame::run() {
    app.initWindow(800, 600, "PixelGame");
    std::vector<Vertex> vertices; std::vector<uint32_t> indices;
    pool.enqueue([this,&vertices,&indices](){ loadWorld(vertices, indices); }).wait();
    app.initVulkan(vertices, indices);
    app.mainLoop();
    app.cleanup();
}
