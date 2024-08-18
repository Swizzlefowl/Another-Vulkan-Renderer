#pragma once
#include <vulkan/vulkan.hpp>
#include <fmt/core.h>
#include "context.h"
namespace avr {
    class Image {
    public:
        Image();
        ~Image();
        vk::Image vkImage{};
        vk::ImageView view{};
    };
}

