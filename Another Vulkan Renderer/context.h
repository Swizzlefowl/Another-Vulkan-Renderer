#pragma once
#include <string>
#include "GLFW/glfw3.h"
#include <vulkan/vulkan.hpp>
class Context{
public:
    class Window {
    public:
        size_t height{};
        size_t width{};
        GLFWwindow* window{ nullptr };
        Window();
        Window(size_t height, size_t width, const std::string& title);
        Window(Window&& win);
        Window& operator=(Window&& win);
        ~Window();
    };
    Context();
    void createWindow(size_t height, size_t width, const std::string& title);
    GLFWwindow* getWindow() const;
    Window window{};

    void initVulkanCtx();

private:
    vk::Instance instance{};

    void createVkInstance();
};

