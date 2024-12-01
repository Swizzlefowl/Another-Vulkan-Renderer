#include "Image.hpp"
namespace avr {
    imageBuilder::imageBuilder(){
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.extent.width = static_cast<uint32_t>(1);
        imageInfo.extent.height = static_cast<uint32_t>(1);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = vk::Format::eR8G8B8A8Srgb;
        imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
    }

    avr::Image imageBuilder::createImage(const Context& ctx, VmaAllocationCreateFlags flags) {
        vk::Image image{};
        VmaAllocation allocation{};

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = flags;
        auto result = vmaCreateImage(ctx.allocator, reinterpret_cast<VkImageCreateInfo*>(&imageInfo), &allocInfo, reinterpret_cast<VkImage*>(&image), &allocation, nullptr);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("image creation failed");
        }
        return avr::Image{ .image = image, .alloc = allocation, .view = {},
            .format = imageInfo.format, .width = imageInfo.extent.width,
            .height = imageInfo.extent.height,
            .mips = imageInfo.mipLevels,
            .layers = imageInfo.arrayLayers,
            .baseMip = {},
            .baseLayer = {}
        };
    }

    imageBuilder& imageBuilder::setWidth(u32 width){
        imageInfo.extent.width = width;
        return *this;
    }

    imageBuilder& imageBuilder::setHeight(u32 height){
        imageInfo.extent.height = height;
        return *this;
    }
    imageBuilder& imageBuilder::setMips(u32 mips){
        imageInfo.mipLevels = mips;
        return *this;
    }

    imageBuilder& imageBuilder::setArrayLayers(u32 arrayLayers){
        imageInfo.arrayLayers = arrayLayers;
        return *this;
    }

    imageBuilder& imageBuilder::setFormat(vk::Format format){
        imageInfo.format = format;
        return *this;
    }

    imageBuilder& imageBuilder::setUsage(vk::ImageUsageFlags usage){
        imageInfo.usage = usage;
        return *this;
    }

    imageBuilder& imageBuilder::setTiling(vk::ImageTiling tiling){
        imageInfo.tiling = tiling;
        return *this;
    }
    ImageViewBuilder::ImageViewBuilder(){
        createInfo.viewType = vk::ImageViewType::e2D;
        vk::ComponentMapping mappings{
            vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity };
        createInfo.components = mappings;

        // base	MipmapLevel = 0, levelcount = 1, baseArrayLayer = 0, layerCount
        // =
        // 1
        vk::ImageSubresourceRange imageSubResource{vk::ImageAspectFlagBits::eColor,
        0, 1, 0, 1 };
        createInfo.subresourceRange = imageSubResource;
    }

    vk::ImageView ImageViewBuilder::createImageView(Context& ctx){
        try {
            return ctx.device.createImageView(createInfo);
        }
        catch (const vk::SystemError& err) {
            throw std::runtime_error(err.what());
        }
    }

    ImageViewBuilder& ImageViewBuilder::setImage(vk::Image image){
        createInfo.image = image;
        return *this;
    }

    ImageViewBuilder& ImageViewBuilder::setViewType(vk::ImageViewType viewType){
        createInfo.viewType = viewType;
        return *this;
    }

    ImageViewBuilder& ImageViewBuilder::setAspect(vk::ImageAspectFlags aspect){
        createInfo.subresourceRange.aspectMask = aspect;
        return *this;
    }

    ImageViewBuilder& ImageViewBuilder::setMips(u32 baseLevel, u32 mipsCount){
        createInfo.subresourceRange.baseMipLevel = baseLevel;
        createInfo.subresourceRange.levelCount = mipsCount;
        return *this;
    }

    ImageViewBuilder& ImageViewBuilder::setArrayLayers(u32 baselayer, u32 arrayCount){
        createInfo.subresourceRange.baseArrayLayer = baselayer;
        createInfo.subresourceRange.layerCount = arrayCount;
        return *this;
    }

    ImageViewBuilder& ImageViewBuilder::setFormat(vk::Format format){
        createInfo.format = format;
        return *this;
    }

}
