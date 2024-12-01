#pragma once
#include <vulkan/vulkan.hpp>
#include <fmt/core.h>
#include "context.h"
namespace avr {
    using u64 = std::uint64_t;
    using u32 = std::uint32_t;
    class Image {
    public:
        vk::Image image{};
        VmaAllocation alloc{};
        vk::ImageView view{};
        vk::Format format{};
        u32 width{};
        u32 height{};
        u32 mips{};
        u32 layers{};
        u32 baseMip{};
        u32 baseLayer{};
        u64 texIndex{};
    };

    class imageBuilder {
    public:
        imageBuilder();
        avr::Image createImage(const Context& ctx, VmaAllocationCreateFlags flags);
        imageBuilder& setWidth(u32 width);
        imageBuilder& setHeight(u32 height);
        imageBuilder& setMips(u32 mips);
        imageBuilder& setArrayLayers(u32 arrayLayers);
        imageBuilder& setFormat(vk::Format format);
        imageBuilder& setUsage(vk::ImageUsageFlags usage);
        imageBuilder& setTiling(vk::ImageTiling tiling);
    private:
        vk::ImageCreateInfo imageInfo{};
    };

    class ImageViewBuilder {
    public:
        ImageViewBuilder();
        vk::ImageView createImageView(Context& ctx);
        ImageViewBuilder& setImage(vk::Image image);
        ImageViewBuilder& setViewType(vk::ImageViewType viewType);
        ImageViewBuilder& setAspect(vk::ImageAspectFlags aspect);
        ImageViewBuilder& setMips(u32 baseLevel,u32 mipsCount);
        ImageViewBuilder& setArrayLayers(u32 baselayer, u32 arrayCount);
        ImageViewBuilder& setFormat(vk::Format format);

    private:
        vk::ImageViewCreateInfo createInfo{};
        vk::ComponentMapping mappings{};
    };
}

