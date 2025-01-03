#include "engineUtils.h"
#include <fstream>
#include "stb_image.h"

namespace avr {
    vk::ImageView createImageView(Context& ctx, vk::Image image, vk::Format format, vk::ImageSubresourceRange range, vk::ImageViewType viewType) {
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

    vk::CommandPool createCommandPool(Context& ctx) {
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

    vk::CommandBuffer createCommandBuffer(Context& ctx, vk::CommandPool& pool) {
        vk::CommandBufferAllocateInfo allocateInfo{};
        allocateInfo.commandPool = pool;
        allocateInfo.level = vk::CommandBufferLevel::ePrimary;
        allocateInfo.commandBufferCount = 1;
        return ctx.device.allocateCommandBuffers(allocateInfo)[0];
    }
    std::vector<vk::CommandBuffer> createCommandBuffer(Context& ctx, vk::CommandPool& pool, uint32_t count) {
        vk::CommandBufferAllocateInfo allocateInfo{};
        allocateInfo.commandPool = pool;
        allocateInfo.level = vk::CommandBufferLevel::ePrimary;
        allocateInfo.commandBufferCount = count;
        return ctx.device.allocateCommandBuffers(allocateInfo);
    }
    vk::ShaderModule createShader(Context& ctx, const std::string& fileName) {
        // remember to do a bitwise or operation between the flags instead of
   // adding a comma....
        std::ifstream file{ fileName, std::ios::ate | std::ios::binary };

        if (!file.is_open())
            throw std::runtime_error("failed to open file");

        size_t fileSize{ static_cast<size_t>(file.tellg()) };
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        vk::ShaderModuleCreateInfo createInfo{};
        createInfo.codeSize = buffer.size();
        createInfo.pCode = reinterpret_cast<uint32_t*>(buffer.data());

        try {
            return ctx.device.createShaderModule(createInfo);
        }
        catch (vk::Error& err) {
            err.what();
        }
        return vk::ShaderModule{};
    }

    vk::Pipeline createPipeline(Context& ctx, vk::PipelineLayout pipeLayout, vk::DescriptorSetLayout setLayout, const Shaders& shaders, const std::vector<vk::Format> formats) {

        vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
        vertShaderStageInfo.module = shaders.vertShader;
        vertShaderStageInfo.pName = "main";

        vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
        fragShaderStageInfo.module = shaders.fragShader;
        fragShaderStageInfo.pName = "main";

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStagesInfo{ vertShaderStageInfo,
            fragShaderStageInfo };

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        vk::PipelineViewportStateCreateInfo viewportState{};
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        vk::PipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = vk::PolygonMode::eFill;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = vk::CullModeFlagBits::eBack;
        rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;

        vk::PipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        colorBlendAttachment.blendEnable = VK_FALSE;

        vk::PipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = vk::LogicOp::eCopy;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<vk::DynamicState> dynamicStates{
            vk::DynamicState::eViewport, vk::DynamicState::eScissor };

        vk::PipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        vk::PipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = vk::CompareOp::eLess;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE; // Optional
    
        vk::GraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.stageCount = shaderStagesInfo.size();
        pipelineInfo.pStages = shaderStagesInfo.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipeLayout;
        pipelineInfo.renderPass = VK_NULL_HANDLE;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;
        vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
        pipelineRenderingCreateInfo.colorAttachmentCount = formats.size();
        pipelineRenderingCreateInfo.pColorAttachmentFormats = formats.data();
        pipelineRenderingCreateInfo.depthAttachmentFormat = vk::Format::eD32Sfloat;
        // Chain into the pipeline create info
        pipelineInfo.pNext = &pipelineRenderingCreateInfo;
        vk::Pipeline pipe{};
        try {
            pipe = ctx.device.createGraphicsPipeline(nullptr, pipelineInfo).value;
        }
        catch (vk::SystemError& err) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
        fmt::println("created pipeline");
        ctx.device.destroyShaderModule(shaders.vertShader);
        ctx.device.destroyShaderModule(shaders.fragShader);
        return pipe;
    }
    vk::Semaphore createVKSemaphore(Context& ctx) {
        vk::SemaphoreCreateInfo semaphoreInfo{};
        try {
            return ctx.device.createSemaphore(semaphoreInfo);
        }
        catch (vk::Error& err) {
            throw std::runtime_error(err.what());
        }
    }

    vk::Fence createVKFence(Context& ctx) {
        vk::FenceCreateInfo fenceInfo;
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
        try {
            return ctx.device.createFence(fenceInfo);
        }
        catch (vk::Error& err) {
            throw std::runtime_error(err.what());
        }
    }
    void transitionLayout(vk::CommandBuffer& cb, const std::vector<vk::ImageMemoryBarrier2>& barriers) {
        vk::DependencyInfo info{};
        info.imageMemoryBarrierCount = barriers.size();
        info.pImageMemoryBarriers = barriers.data();
        cb.pipelineBarrier2(info);
    }
    void transitionLayout(vk::CommandBuffer& cb, const vk::ImageMemoryBarrier2& barrier) {
        vk::DependencyInfo info{};
        info.imageMemoryBarrierCount = 1;
        info.pImageMemoryBarriers = &barrier;
        cb.pipelineBarrier2(info);
    }
    vk::Buffer createBuffer(Context& ctx, const VmaAllocationCreateInfo& info, vk::BufferUsageFlags usage, vk::DeviceSize size, VmaAllocation& allocation) {
        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        vk::Buffer buffer{};
        auto result = vmaCreateBuffer(ctx.allocator, reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo), &info, reinterpret_cast<VkBuffer*>(&buffer), &allocation, nullptr);

        if (result != VkResult::VK_SUCCESS)
            throw std::runtime_error("creating buffer failed");
        else
            return buffer;
    }
    void mapMemory(const Context& ctx, const VmaAllocation& allocation, void* src, VkDeviceSize size){
        void* stagingMappedPtr{ nullptr };
        auto result = vmaMapMemory(ctx.allocator, allocation, &stagingMappedPtr);

        if (result != VkResult::VK_SUCCESS)
            throw std::runtime_error("map memory failed");

        std::memcpy(stagingMappedPtr, src, size);
        result = vmaFlushAllocation(ctx.allocator, allocation, 0, size);

        if (result != VkResult::VK_SUCCESS)
            throw std::runtime_error("vma flush failed");
        vmaUnmapMemory(ctx.allocator, allocation);
    }

    vk::CommandBuffer createSingleTimeCB(Context& ctx){
        auto cb = createCommandBuffer(ctx, ctx.commandPool);
        vk::CommandBufferBeginInfo beginInfo{};
        cb.begin(beginInfo);
        return cb;
    }

    void copyBufferToImage(vk::CommandBuffer cb, vk::Buffer buffer, vk::Image image){
        
    }
    void execute(Context& ctx, vk::CommandBuffer cb, std::function<void()>&& fn){
        fn();
        cb.end();
        vk::SubmitInfo submitInfo{};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cb;
        ctx.queue.submit(submitInfo);
        ctx.queue.waitIdle();
    }

    void createTexture(Context& ctx, std::string_view name, avr::Image& image){
        int texWidth{};
        int texHeight{};
        int texChannels{};
        stbi_uc* pixels{ nullptr };
        pixels = stbi_load(name.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        vk::DeviceSize imageSize{ static_cast<vk::DeviceSize>(texWidth * texHeight * 4) };

        if (!pixels)
            throw std::runtime_error("failed to load image!");
        imageBuilder builder{};
        image = builder.setWidth(texWidth)
            .setHeight(texHeight)
            .setMips(1)
            .setArrayLayers(1)
            .setTiling(vk::ImageTiling::eOptimal)
            .setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
            .setFormat(vk::Format::eR8G8B8A8Srgb)
            .createImage(ctx, 0);

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        VmaAllocation stagingAlloc{};
        auto stagingBuffer = createBuffer(ctx, allocInfo, vk::BufferUsageFlagBits::eTransferSrc, imageSize, stagingAlloc);
        mapMemory(ctx, stagingAlloc, pixels, imageSize);

        auto cb = createSingleTimeCB(ctx);
        execute(ctx, cb, [&]() {
            vk::BufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            region.imageSubresource.mipLevel = image.baseMip;
            region.imageSubresource.baseArrayLayer = image.baseLayer;
            region.imageSubresource.layerCount = image.layers;

            region.imageOffset = vk::Offset3D{ 0, 0, 0 };
            region.imageExtent = vk::Extent3D{
                image.width,
                image.height,
                1 };

            BarrierBuilder builder{};
            auto barrier = builder.setImage(image.image)
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                .setSubresourceRange(vk::ImageSubresourceRange{
                vk::ImageAspectFlagBits::eColor
                    ,0, 1, 0, 1 })
                .setSrc(vk::PipelineStageFlagBits2::eNone, vk::AccessFlagBits2::eNone)
                .setDst(vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite)
                .createImageBarrier();
            transitionLayout(cb, barrier);
            cb.copyBufferToImage(stagingBuffer, image.image, vk::ImageLayout::eTransferDstOptimal, region);

            auto barrier2 = builder.setImage(image.image)
                .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setSubresourceRange(vk::ImageSubresourceRange{
                vk::ImageAspectFlagBits::eColor
                    ,0, 1, 0, 1 })
                .setSrc(vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite)
                .setDst(vk::PipelineStageFlagBits2::eFragmentShader, vk::AccessFlagBits2::eShaderRead)
                .createImageBarrier();
            transitionLayout(cb, barrier2);
            });

        ImageViewBuilder viewBuilder{};
        image.view = viewBuilder.setImage(image.image)
            .setFormat(image.format)
            .setAspect(vk::ImageAspectFlagBits::eColor)
            .setMips(0, 1)
            .setArrayLayers(0, 1)
            .setViewType(vk::ImageViewType::e2D)
            .createImageView(ctx);

        image.texIndex = ctx.descManager.write(vk::ImageLayout::eShaderReadOnlyOptimal ,vk::DescriptorType::eSampledImage, image.view);
        vmaDestroyBuffer(ctx.allocator, stagingBuffer, stagingAlloc);
        stbi_image_free(pixels);
    }

    BarrierBuilder& BarrierBuilder::setOldLayout(vk::ImageLayout layout){
        imageBarrier.oldLayout = layout;
        return *this;
    }

    BarrierBuilder& BarrierBuilder::setNewLayout(vk::ImageLayout layout){
        imageBarrier.newLayout = layout;
        return *this;
    }

    BarrierBuilder& BarrierBuilder::setAspect(vk::ImageAspectFlags aspect){
        imageBarrier.subresourceRange.aspectMask = aspect;
        return *this;
    }

    BarrierBuilder& BarrierBuilder::setImage(vk::Image image){
        imageBarrier.image = image;
        return *this;
    }

    BarrierBuilder& BarrierBuilder::setSubresourceRange(vk::ImageSubresourceRange range){
        imageBarrier.subresourceRange = range;
        return *this;
    }
    BarrierBuilder& BarrierBuilder::setSrc(vk::PipelineStageFlags2 stageFlags, vk::AccessFlags2 accessFlags){
        imageBarrier.srcStageMask = stageFlags;
        imageBarrier.srcAccessMask = accessFlags;
        return *this;
    }

    BarrierBuilder& BarrierBuilder::setDst(vk::PipelineStageFlags2 stageFlags, vk::AccessFlags2 accessFlags){
        imageBarrier.dstStageMask = stageFlags;
        imageBarrier.dstAccessMask = accessFlags;
        return *this;
    }

    vk::ImageMemoryBarrier2 BarrierBuilder::createImageBarrier(){
        imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        return imageBarrier;
    }
}