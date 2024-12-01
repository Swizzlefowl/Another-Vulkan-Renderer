#pragma once
#include <vulkan/vulkan.hpp>
#include <fmt/core.h>
#include <vector>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
    struct Vertex {
        glm::vec3 pos{};
        glm::vec3 color{};
        glm::vec2 texCoord{};
    };
    struct BarrierBuilder {
        vk::ImageMemoryBarrier2 imageBarrier{};
        vk::ImageSubresourceRange imageSubResource{};

        BarrierBuilder& setOldLayout(vk::ImageLayout layout);
        BarrierBuilder& setNewLayout(vk::ImageLayout layout);
        BarrierBuilder& setAspect(vk::ImageAspectFlags aspect);
        BarrierBuilder& setImage(vk::Image image);
        BarrierBuilder& setSubresourceRange(vk::ImageSubresourceRange range);
        // src refers to src stage and access flags
        BarrierBuilder& setSrc(vk::PipelineStageFlags2 stageFlags, vk::AccessFlags2 accessFlags);
        // dst refers to dst stage and access flags
        BarrierBuilder& setDst(vk::PipelineStageFlags2 stageFlags, vk::AccessFlags2 accessFlags);
        vk::ImageMemoryBarrier2 createImageBarrier();
    };
    vk::ImageView createImageView(Context& ctx, vk::Image image, vk::Format format, vk::ImageSubresourceRange range, vk::ImageViewType viewType);
    vk::CommandPool createCommandPool(Context& ctx);
    vk::CommandBuffer createCommandBuffer(Context& ctx, vk::CommandPool& pool);
    std::vector<vk::CommandBuffer> createCommandBuffer(Context& ctx, vk::CommandPool& pool, uint32_t count);
    vk::ShaderModule createShader(Context& ctx, const std::string& fileName);
    vk::Pipeline createPipeline(Context& ctx, vk::PipelineLayout pipeLayout, vk::DescriptorSetLayout setLayout, const Shaders& shaders, const std::vector<vk::Format> formats);
    vk::Semaphore createVKSemaphore(Context& ctx);
    vk::Fence createVKFence(Context& ctx);
    void transitionLayout(vk::CommandBuffer& cb, const std::vector<vk::ImageMemoryBarrier2>& barriers);
    void transitionLayout(vk::CommandBuffer& cb, const vk::ImageMemoryBarrier2& barrier);
    vk::Buffer createBuffer(Context& ctx, const VmaAllocationCreateInfo& info, vk::BufferUsageFlags usage, vk::DeviceSize size, VmaAllocation& allocation);
    void mapMemory(const Context& ctx, const VmaAllocation& allocation, void* src, VkDeviceSize size);
    vk::CommandBuffer createSingleTimeCB(Context& ctx);
    void copyBufferToImage(vk::CommandBuffer cb, vk::Buffer buffer, vk::Image image);
    void execute(Context& ctx, vk::CommandBuffer cb ,std::function<void()>&& fn);
}
