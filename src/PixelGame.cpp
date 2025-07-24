#include "PixelGame.h"
#include "BlockRegistry.h"
#include <iostream>

PixelGame::PixelGame() : pool(std::thread::hardware_concurrency()) {
    if (BlockRegistry::count() == 0) {
        BlockRegistry::registerBlock("Air", false);   // id 0
        BlockRegistry::registerBlock("Dirt", true);   // id 1
    }
}
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
    app.setUpdateCallback([this](float dt){ player.update(app.getWindow(), dt); });
    app.mainLoop();
    app.cleanup();
}
