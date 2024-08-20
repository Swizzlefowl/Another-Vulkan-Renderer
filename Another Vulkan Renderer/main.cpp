#include <iostream>
#include "Renderer.hpp"
int main() {
    try {
        avr::Renderer renderer{};
        renderer.init(720, 640, "yet another rectangle");
        renderer.mainLoop();
    }
    catch (std::exception& except) {
        std::cerr << except.what();
    }
    return 0;
}