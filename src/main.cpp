#include "PixelGame.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>

int main() {
    PixelGame game;
    try {
        game.run();
    }
    catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
