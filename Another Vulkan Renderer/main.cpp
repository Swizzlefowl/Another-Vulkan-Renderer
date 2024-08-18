#include <iostream>
#include "Renderer.hpp"
int main() {
    try {
        avr::Renderer renderer{};
        renderer.init();
        while (!glfwWindowShouldClose(renderer.ctx.getWindow())) {
            glfwPollEvents();
        }
    }
    catch (std::exception& except) {
        std::cerr << except.what();
    }
    return 0;
}