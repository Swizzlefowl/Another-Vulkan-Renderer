#include "engineUtils.h"
namespace avr {
    vk::ImageView createImageView(Context& ctx, vk::Image image, vk::Format format, vk::ImageSubresourceRange range){
        vk::ImageViewCreateInfo createInfo{};
        createInfo.image = image;
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.format = format;

        vk::ComponentMapping mappings{
            vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity };
        createInfo.components = mappings;
        createInfo.subresourceRange = range;

        return ctx.device.createImageView(createInfo);
    }

    vk::CommandPool createCommandPool(Context& ctx){
        vk::CommandPoolCreateInfo poolInfo{};
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        poolInfo.queueFamilyIndex = ctx.getQueueIndex(ctx.physicalDevice, QueueTypes::Graphics).first;
        try {
            return ctx.device.createCommandPool(poolInfo);
        }
        catch (vk::SystemError& err) {
            throw std::runtime_error(err.what());
        }
    }

    vk::CommandBuffer createCommandBuffer(Context& ctx, vk::CommandPool& pool){
        vk::CommandBufferAllocateInfo allocateInfo{};
        allocateInfo.commandPool = pool;
        allocateInfo.level = vk::CommandBufferLevel::ePrimary;
        allocateInfo.commandBufferCount = 1;
        return ctx.device.allocateCommandBuffers(allocateInfo)[0];
    }
    std::vector<vk::CommandBuffer> createCommandBuffer(Context& ctx, vk::CommandPool& pool, uint32_t count){
        vk::CommandBufferAllocateInfo allocateInfo{};
        allocateInfo.commandPool = pool;
        allocateInfo.level = vk::CommandBufferLevel::ePrimary;
        allocateInfo.commandBufferCount = count;
        return ctx.device.allocateCommandBuffers(allocateInfo);
    }
}