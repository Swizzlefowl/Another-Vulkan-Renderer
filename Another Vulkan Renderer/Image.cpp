#include "Image.hpp"
namespace avr {
    vk::ImageView Image::createImageView(Context& ctx, vk::Image image, vk::Format format, vk::ImageSubresourceRange range) {
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
}
