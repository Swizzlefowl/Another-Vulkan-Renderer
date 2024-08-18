#pragma once
#include <vulkan/vulkan.hpp>
#include <fmt/core.h>
#include <vector>
#include "context.h"

namespace avr {
    vk::ImageView createImageView(Context& ctx, vk::Image image, vk::Format format, vk::ImageSubresourceRange range);
    vk::CommandPool createCommandPool(Context& ctx);
    vk::CommandBuffer createCommandBuffer(Context& ctx, vk::CommandPool& pool);
    std::vector<vk::CommandBuffer> createCommandBuffer(Context& ctx, vk::CommandPool& pool, uint32_t count);
}
