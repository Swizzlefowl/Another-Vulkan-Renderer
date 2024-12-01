#include "DescriptorManager.hpp"
#include "context.h"
#include "fmt/core.h"
#include <span>
namespace avr {
    using u64 = std::uint64_t;

    DescriptorManager::DescriptorManager(Context& other) :ctx{ other } {
    }
    void DescriptorManager::initManager() {
        vk::DescriptorPoolCreateInfo poolInfo{};
        poolInfo.maxSets = 1;
        poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;

        std::array<vk::DescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = vk::DescriptorType::eSampler;
        poolSizes[0].descriptorCount = 10;
        poolSizes[1].type = vk::DescriptorType::eSampledImage;
        poolSizes[1].descriptorCount = 20;
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        descPool = ctx.device.createDescriptorPool(poolInfo);

        std::array<vk::DescriptorSetLayoutBinding, 2> bindings{};
        vk::DescriptorSetLayoutCreateInfo createInfo{};
        createInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;

        bindings[0].binding = 0;
        bindings[0].descriptorCount = 5;
        bindings[0].descriptorType = vk::DescriptorType::eSampler;
        bindings[0].stageFlags = vk::ShaderStageFlagBits::eFragment;
        bindings[0].pImmutableSamplers = nullptr;

        bindings[1].binding = 1;
        bindings[1].descriptorCount = 10;
        bindings[1].descriptorType = vk::DescriptorType::eSampledImage;
        bindings[1].stageFlags = vk::ShaderStageFlagBits::eFragment;
        bindings[1].pImmutableSamplers = nullptr;

        vk::DescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
        std::array<vk::DescriptorBindingFlags, 2> bindingFlags{};
        
        bindingFlags[0] = vk::DescriptorBindingFlagBits::ePartiallyBound
            | vk::DescriptorBindingFlagBits::eUpdateAfterBind
            | vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending;
        bindingFlags[1] = vk::DescriptorBindingFlagBits::ePartiallyBound
            | vk::DescriptorBindingFlagBits::eUpdateAfterBind
            | vk::DescriptorBindingFlagBits::eVariableDescriptorCount
            | vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending;

        flagsInfo.bindingCount = bindingFlags.size();
        flagsInfo.pBindingFlags = bindingFlags.data();
        createInfo.bindingCount = bindings.size();
        createInfo.pBindings = bindings.data();
        createInfo.pNext = &flagsInfo;
        setLayout = ctx.device.createDescriptorSetLayout(createInfo);

        vk::DescriptorSetAllocateInfo allocateInfo{};
        vk::DescriptorSetVariableDescriptorCountAllocateInfo setVarCount{};
        std::uint32_t count{ 10 };
        setVarCount.descriptorSetCount = 1;
        setVarCount.pDescriptorCounts = &count;
        allocateInfo.descriptorSetCount = 1;
        allocateInfo.descriptorPool = descPool;
        allocateInfo.pSetLayouts = &setLayout;
        allocateInfo.pNext = &setVarCount;

        set = ctx.device.allocateDescriptorSets(allocateInfo)[0];

        ctx.deleteQueue.enqueue(
            [&]() {
                ctx.device.destroyDescriptorPool(descPool);
                ctx.device.destroyDescriptorSetLayout(setLayout);
                fmt::println("destroyed global set");
            }
        );
    }

    std::uint64_t DescriptorManager::write(vk::ImageLayout layout, vk::DescriptorType type, const vk::ImageView view){
            vk::DescriptorImageInfo info{};
            info.imageView = view;
            info.imageLayout = layout;
            vk::WriteDescriptorSet write{};
            write.descriptorType = type;
            write.descriptorCount = 1;
            write.dstBinding = 1;
            write.dstSet = set;
            write.dstArrayElement = index;
            write.pImageInfo = &info;
            ctx.device.updateDescriptorSets(write, nullptr);
            return index++;
    }

    std::uint64_t DescriptorManager::write(vk::DescriptorType type, const vk::Sampler sampler){
            vk::DescriptorImageInfo info{};
            info.sampler = sampler;
            vk::WriteDescriptorSet write{};
            write.descriptorType = type;
            write.descriptorCount = 1;
            write.dstBinding = 0;
            write.dstSet = set;
            write.dstArrayElement = 0;
            write.pImageInfo = &info;
            ctx.device.updateDescriptorSets(write, nullptr);
            return 0;
    }

}