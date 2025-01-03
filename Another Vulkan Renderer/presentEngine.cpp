#include "presentEngine.hpp"
#include <fmt/color.h>
#include "engineUtils.h"
avr::PresentEngine::PresentEngine(Context& other) : ctx{other} {
}

avr::PresentEngine::~PresentEngine(){
    cleanupSwapchain();
}

void avr::PresentEngine::createSwapchain(){
    SwapChainCapablities swapchainCapabilites{ getSwapChainCapabilities() };
    vk::SurfaceFormatKHR surfaceFormat{ chooseSwapSurfaceFormat(swapchainCapabilites.formats) };
    vk::PresentModeKHR presentMode{ chooseSwapPresentMode(swapchainCapabilites.presentMode) };
    vk::Extent2D extent{ chooseSwapExtend(swapchainCapabilites.capabilities) };
    uint32_t imageCount{ swapchainCapabilites.capabilities.minImageCount + 1 };

    if (swapchainCapabilites.capabilities.maxImageCount > 0 && imageCount > swapchainCapabilites.capabilities.maxImageCount)
        imageCount = swapchainCapabilites.capabilities.maxImageCount;

    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.surface = ctx.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;

    auto queueFamilyIndex = ctx.getQueueIndex(ctx.physicalDevice, QueueTypes::Graphics);
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    createInfo.queueFamilyIndexCount = queueFamilyIndex.first;
    createInfo.pQueueFamilyIndices = nullptr;

    createInfo.preTransform = swapchainCapabilites.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    try {
        swapchain = ctx.device.createSwapchainKHR(createInfo);
    }
    catch (vk::Error& err) {
        throw std::runtime_error("failed to create swap chain!");
    }
    swapChainImagesFormat = surfaceFormat.format;
    swapChainExtent = extent;
    fmt::println("created swapchain");
    return;
}

void avr::PresentEngine::createSwapchainImageViews(){
    swapchainImages = ctx.device.getSwapchainImagesKHR(swapchain);
    swapchainImageViews.resize(swapchainImages.size());

    size_t i{ 0 };
    for (auto& imageView : swapchainImageViews) {
        vk::ImageSubresourceRange imageSubResource{ vk::ImageAspectFlagBits::eColor,
         0, 1, 0, 1 };
        imageView = avr::createImageView(ctx, swapchainImages[i], swapChainImagesFormat, imageSubResource, vk::ImageViewType::e2D);
        i++;
    }
    fmt::println("created swpachain image views");
    return;
}

void avr::PresentEngine::cleanupSwapchain(){
    ctx.device.destroySwapchainKHR(swapchain);
    fmt::println("swapchain destroyed");
    for (auto& view : swapchainImageViews) {
        ctx.device.destroyImageView(view);
    }
    fmt::println("swapchain image views destroyed");
}

avr::PresentEngine::SwapChainCapablities avr::PresentEngine::getSwapChainCapabilities(){
    return SwapChainCapablities{
       .capabilities = ctx.physicalDevice.getSurfaceCapabilitiesKHR(ctx.surface),
       .formats = ctx.physicalDevice.getSurfaceFormatsKHR(ctx.surface),
       .presentMode = ctx.physicalDevice.getSurfacePresentModesKHR(ctx.surface) };
}

vk::SurfaceFormatKHR avr::PresentEngine::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> availableFormats){
    for (const auto& availableFormat : availableFormats)
        if (availableFormat.format == vk::Format::eR8G8B8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            return availableFormat;
    return availableFormats[0];
}

vk::PresentModeKHR avr::PresentEngine::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes){
    for (const auto& presentMode : availablePresentModes)
        if (presentMode == vk::PresentModeKHR::eMailbox)
            return presentMode;
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D avr::PresentEngine::chooseSwapExtend(const vk::SurfaceCapabilitiesKHR& capabilities){
    if (capabilities.currentExtent != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(ctx.getWindow(), &width, &height);

        vk::Extent2D actualExtent{ static_cast<uint32_t>(width),
            static_cast<uint32_t>(height) };
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width);

        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height);

        return actualExtent;
    }
    return vk::Extent2D();
}
