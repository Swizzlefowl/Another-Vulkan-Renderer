#include <GLFW/glfw3.h>
#include "context.h"
#include <iostream>
int main() {
    try {
        Context ctx{};
        ctx.createWindow(720, 680, "another triangle");
        ctx.initVulkanCtx();
        while (!glfwWindowShouldClose(ctx.getWindow())) {
            glfwPollEvents();
        }
    }
    catch (std::exception& except) {
        std::cerr << except.what();
    }
    return 0;
}