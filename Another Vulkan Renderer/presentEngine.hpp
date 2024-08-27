#pragma once
#include "context.h"
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
namespace avr {
    class PresentEngine {

        struct SwapChainCapablities {
            vk::SurfaceCapabilitiesKHR capabilities;
            std::vector<vk::SurfaceFormatKHR> formats;
            std::vector<vk::PresentModeKHR> presentMode;
        };
    public:
        PresentEngine(Context& other);
        ~PresentEngine();
        void createSwapchain();
        void createSwapchainImageViews();
        void cleanupSwapchain();
        Context& ctx;
        vk::SwapchainKHR swapchain{};
        vk::Format swapChainImagesFormat{};
        vk::Extent2D swapChainExtent{};
        std::vector<vk::Image> swapchainImages{};
        std::vector<vk::ImageView> swapchainImageViews{};
      
    private:
        SwapChainCapablities getSwapChainCapabilities();
        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<vk::SurfaceFormatKHR> availableFormats);
        vk::PresentModeKHR chooseSwapPresentMode(
            const std::vector<vk::PresentModeKHR> availablePresentModes);
        vk::Extent2D chooseSwapExtend(const vk::SurfaceCapabilitiesKHR& capabilities);
    };
}
