#include "VulkanApp.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>

int main() {
    VulkanApp app;
    try {
        app.initWindow(800, 600, "VoxelDemo");
        app.initVulkan();
        app.mainLoop();
        app.cleanup();
    }
    catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        std::cerr << "Press Enter to exit..." << std::endl;
        std::cin.get();   // ← waits here so you can read the message
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
