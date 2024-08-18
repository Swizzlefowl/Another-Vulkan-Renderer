#pragma once
#include <vulkan/vulkan.hpp>
#include <fmt/core.h>
#include "context.h"
namespace avr {
    class Image {
    public:
        static vk::ImageView createImageView(Context& ctx,vk::Image image, vk::Format format, vk::ImageSubresourceRange range);
    };
}

