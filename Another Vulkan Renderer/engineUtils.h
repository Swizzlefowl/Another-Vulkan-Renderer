#pragma once
#include <vulkan/vulkan.hpp>
#include <fmt/core.h>
#include <vector>
#include "context.h"
namespace avr {
    enum class pipeLineType {
        Graphics,
        Compute,
    };
    struct Shaders {
        pipeLineType type{};
        vk::ShaderModule vertShader{};
        vk::ShaderModule fragShader{};
        vk::ShaderModule computeShader{}; //maybe unused depending on the type
    };
    vk::ImageView createImageView(Context& ctx, vk::Image image, vk::Format format, vk::ImageSubresourceRange range);
    vk::CommandPool createCommandPool(Context& ctx);
    vk::CommandBuffer createCommandBuffer(Context& ctx, vk::CommandPool& pool);
    std::vector<vk::CommandBuffer> createCommandBuffer(Context& ctx, vk::CommandPool& pool, uint32_t count);
    vk::ShaderModule createShader(Context& ctx, const std::string& fileName);
    vk::Pipeline createPipeline(Context& ctx, vk::PipelineLayout pipeLayout, vk::DescriptorSetLayout setLayout, const Shaders& shaders, const std::vector<vk::Format> formats);
}
