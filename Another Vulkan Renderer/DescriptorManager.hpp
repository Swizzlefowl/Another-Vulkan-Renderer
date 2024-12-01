#pragma once
#include <vulkan/vulkan.hpp>
#include <optional>
#include <cstdint>
namespace avr {
    class Context;

    class DescriptorManager {
        using u64 = std::uint64_t;
    public:
        DescriptorManager(Context& ctx);
        void initManager();
        std::uint64_t write(vk::ImageLayout layout, vk::DescriptorType type, const vk::ImageView view);
        std::uint64_t write(vk::DescriptorType type, const vk::Sampler sampler);
        vk::DescriptorPool descPool{};
        vk::DescriptorSetLayout setLayout{};
        vk::DescriptorSet set{};
        Context& ctx;

    private:
        u64 index{};
    };
}

