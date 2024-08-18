#pragma once
#include <string>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "deletionQueue.h"
namespace avr {
    enum class Levels {
        Instance,
        Device
    };
    enum class QueueTypes {
        Graphics,
        Compute,
        Transfer,
        Invalid
    };
    class Context {
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
        vk::Instance instance{};
        vk::SurfaceKHR surface{};
        vk::PhysicalDevice physicalDevice{};
        vk::Device device{};
        vk::Queue queue{};
        vk::CommandPool commandPool{};
        vk::CommandBuffer commandBuffer{};
        DeletionQueue deleteQueue{};
        void initVulkanCtx();
        std::pair<uint32_t, QueueTypes> getQueueIndex(const vk::PhysicalDevice& dev, QueueTypes type);
    private:
        // @level level refers to instance or devicce level extension
        bool checkForRequiredExtension(const std::string& name, Levels level, vk::PhysicalDevice device = nullptr) const;
        void createVkInstance();
        void createSurface();
        bool isDeviceSuitable(const vk::PhysicalDevice& dev);
        vk::PhysicalDevice getPdevice();
        void createLogicalDevice();
        void createCommandPool();
        void createCommandBuffer();
    };
}

