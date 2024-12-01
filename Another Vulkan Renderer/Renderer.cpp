#include "Renderer.hpp"

namespace avr {
    void Renderer::init() {
        ctx.createWindow(720, 640, "default name");
        ctx.initVulkanCtx();
        pEngine.createSwapchain();
        pEngine.createSwapchainImageViews();
        createSampler();
        preparePipeline(graphicsPipe);
        createSyncObjects();
        createDepthBuffer();
        registerMeshes();
    }

    void Renderer::init(size_t height, size_t width, const std::string& title) {
        ctx.createWindow(height, width, title);
        ctx.initVulkanCtx();
        pEngine.createSwapchain();
        pEngine.createSwapchainImageViews();
        createSampler();
        preparePipeline(graphicsPipe);
        createSyncObjects();
        createDepthBuffer();
        registerMeshes();
    }

    Renderer::Renderer() {
    }

    Renderer::~Renderer() {
    }

    void Renderer::createSampler(){
        vk::PhysicalDeviceProperties deviceProperties = ctx.physicalDevice.getProperties();
        vk::SamplerCreateInfo samplerInfo{};
        samplerInfo.magFilter = vk::Filter::eLinear;
        samplerInfo.minFilter = vk::Filter::eLinear;
        samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = vk::CompareOp::eAlways;
        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        sampler.sample = ctx.device.createSampler(samplerInfo);

        sampler.index = ctx.descManager.write(vk::DescriptorType::eSampler, sampler.sample);
        renderDelQueue.enqueue([&]() {
            ctx.device.destroySampler(sampler.sample);
            fmt::println("destroyed sampler");
            });
    }

    void Renderer::preparePipeline(vk::Pipeline& pipe) {
        std::vector<vk::Format> formats{ pEngine.swapChainImagesFormat };
        Shaders shaders{};
        shaders.type = avr::pipeLineType::Graphics;
        shaders.vertShader = avr::createShader(ctx, "vertex.spv");
        shaders.fragShader = avr::createShader(ctx, "frag.spv");
        vk::PushConstantRange ranges{};
        ranges.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
        ranges.size = 128;
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &ctx.descManager.setLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &ranges;

        try {
            pipeLayout = ctx.device.createPipelineLayout(pipelineLayoutInfo);
        }
        catch (vk::SystemError& err) {
            err.what();
        }

        pipe = avr::createPipeline(ctx, pipeLayout, nullptr, shaders, formats);

        renderDelQueue.enqueue([&]() {
            ctx.device.destroyPipelineLayout(pipeLayout);
            fmt::println("destroyed pipeline layout");
            });
        renderDelQueue.enqueue([&]() {
            ctx.device.destroyPipeline(pipe);
            fmt::println("destroyed pipeline");
            });
    }
    void Renderer::createSyncObjects() {
        for (uint32_t index{}; index < frameInFlight; index++) {
            inFlightFence[index] = avr::createVKFence(ctx);
            aquireSem[index] = avr::createVKSemaphore(ctx);
        }
        finishedRenderSem.resize(pEngine.swapchainImages.size());
        for (uint32_t index{}; index < pEngine.swapchainImages.size(); index++) {
            finishedRenderSem[index] = avr::createVKSemaphore(ctx);
        }

        renderDelQueue.enqueue([&]() {
            ctx.device.waitIdle();
            for(auto& fence : inFlightFence)
                ctx.device.destroyFence(fence);
            for(auto& renderSem : finishedRenderSem)
                ctx.device.destroySemaphore(renderSem);
            for(auto& aqSem : aquireSem)
                ctx.device.destroySemaphore(aqSem);
            fmt::println("destroyed semaphores and fences");
            }
        );
    }

    void Renderer::createVertexBuffer(){
    }

    void Renderer::createDepthBuffer(){
        imageBuilder builder{};
        ImageViewBuilder viewBuilder{};
        depthImage = builder.setWidth(pEngine.swapChainExtent.width)
            .setHeight(pEngine.swapChainExtent.height)
            .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
            .setFormat(vk::Format::eD32Sfloat)
            .createImage(ctx, VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

        depthImage.view = viewBuilder.setImage(depthImage.image)
            .setAspect(vk::ImageAspectFlagBits::eDepth)
            .setFormat(depthImage.format)
            .createImageView(ctx);

        fmt::println("created depth image");
        renderDelQueue.enqueue([&]() {
            vmaDestroyImage(ctx.allocator, depthImage.image, depthImage.alloc);
            ctx.device.destroyImageView(depthImage.view);
            fmt::println("destroyed depth image");
            });
    }

    void Renderer::registerMeshes(){
        mesh.createMesh("viking_room.obj", "viking_room.png");
        renderDelQueue.enqueue([&]() {
            vmaDestroyBuffer(ctx.allocator, mesh.vertBuffer, mesh.vertAlloc);
            vmaDestroyImage(ctx.allocator, mesh.texture.image, mesh.texture.alloc);
            ctx.device.destroyImageView(mesh.texture.view);
            fmt::println("destroyed mesh");
            });
        
    }

    void Renderer::recordCB(vk::CommandBuffer& cBuffer, uint32_t imageIndex){
        vk::CommandBufferBeginInfo beginInfo{};
        cBuffer.begin(beginInfo);

        vk::RenderingInfo rInfo{};
        vk::RenderingAttachmentInfo aInfo{};
        vk::ClearValue clearColor{ {0.0f, 0.0f, 0.0f, 1.0f} };
        aInfo.clearValue = clearColor;
        aInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        aInfo.loadOp = vk::AttachmentLoadOp::eClear;
        aInfo.storeOp = vk::AttachmentStoreOp::eStore;
        aInfo.imageView = pEngine.swapchainImageViews[imageIndex];

        vk::RenderingAttachmentInfo dInfo{};
        vk::ClearValue depthClear{};
        depthClear.depthStencil = vk::ClearDepthStencilValue{ 1.0, 0 };
        dInfo.clearValue = depthClear;
        dInfo.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
        dInfo.loadOp = vk::AttachmentLoadOp::eClear;
        dInfo.storeOp = vk::AttachmentStoreOp::eStore;
        dInfo.imageView = depthImage.view;

        rInfo.colorAttachmentCount = 1;
        rInfo.pColorAttachments = &aInfo;
        rInfo.pDepthAttachment = &dInfo;
        rInfo.layerCount = 1;
        rInfo.renderArea = vk::Rect2D{
            {0, 0}, pEngine.swapChainExtent };

        vk::ImageMemoryBarrier2 barrier{};
        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
        barrier.image = pEngine.swapchainImages[imageIndex];
        vk::ImageSubresourceRange imageSubResource{ vk::ImageAspectFlagBits::eColor,
           0, 1, 0, 1 };
        barrier.subresourceRange = imageSubResource;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.srcAccessMask = vk::AccessFlagBits2::eNone;
        barrier.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
        barrier.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
        barrier.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;

        vk::ImageMemoryBarrier2 depthBarrier{};
        depthBarrier.oldLayout = vk::ImageLayout::eUndefined;
        depthBarrier.newLayout = vk::ImageLayout::eDepthAttachmentOptimal;
        depthBarrier.image = depthImage.image;
        vk::ImageSubresourceRange depthimageSubResource{ vk::ImageAspectFlagBits::eDepth,
           0, 1, 0, 1 };
        depthBarrier.subresourceRange = depthimageSubResource;
        depthBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        depthBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        depthBarrier.srcAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
        depthBarrier.srcStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
        depthBarrier.dstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;;
        depthBarrier.dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite;

        transitionLayout(cBuffer, barrier);
        transitionLayout(cBuffer, depthBarrier);
        cBuffer.beginRendering(rInfo);

        vk::Viewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0;
        viewport.width = static_cast<float>(pEngine.swapChainExtent.width);
        viewport.height = static_cast<float>(pEngine.swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        cBuffer.setViewport(0, viewport);

        vk::Rect2D scissor{};
        scissor.offset = vk::Offset2D{ 0, 0 };
        scissor.extent = pEngine.swapChainExtent;
        cBuffer.setScissor(0, scissor);
        cBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipe);

        struct Push {
            vk::DeviceAddress address{};
            glm::mat4 mvp{};
            u32 index{};
        } ps;
        auto model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(-5, 5, 6), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), pEngine.swapChainExtent.width / (float)pEngine.swapChainExtent.height, 0.1f, 100.0f);
        proj[1][1] *= -1;
        glm::mat4 mvp = proj * view * model;
        ps.mvp = mvp;
        ps.address = mesh.vertexAdress;
        ps.index = mesh.texture.texIndex;
        cBuffer.pushConstants<Push>(pipeLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, ps);
        cBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeLayout, 0, ctx.descManager.set, nullptr);
        cBuffer.draw(mesh.vertCount, 1, 0, 0);
        cBuffer.endRendering();
        vk::ImageMemoryBarrier2 presentBarrier{};
        presentBarrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
        presentBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
        presentBarrier.image = pEngine.swapchainImages[imageIndex];
        vk::ImageSubresourceRange imageSubResource2{ vk::ImageAspectFlagBits::eColor,
           0, 1, 0, 1 };
        presentBarrier.subresourceRange = imageSubResource2;
        presentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        presentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        presentBarrier.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
        presentBarrier.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
        presentBarrier.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
        presentBarrier.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite;
        transitionLayout(cBuffer, presentBarrier);

        try {
            cBuffer.end();
        }
        catch (vk::SystemError err) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
    void Renderer::mainLoop(){
        while (!glfwWindowShouldClose(ctx.getWindow())) {
            glfwPollEvents();
            drawFrame();
        }
        ctx.device.waitIdle();
    }
    void Renderer::drawFrame(){
        ctx.device.waitForFences(inFlightFence[frame], VK_FALSE, UINT64_MAX);
        vk::Result result;
        uint32_t imageIndex{};
            std::tie(result, imageIndex) = ctx.device.acquireNextImageKHR(pEngine.swapchain,
                UINT64_MAX, aquireSem[frame]
            );
        ctx.device.resetFences(inFlightFence[frame]);

        ctx.commandBuffer[frame].reset();
        recordCB(ctx.commandBuffer[frame], imageIndex);
        //recordComputeCB(pResources->commandBuffer[0], imageIndex);
        vk::SubmitInfo submitInfo{};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &aquireSem[frame];
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &ctx.commandBuffer[frame];
        vk::PipelineStageFlags waitStages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
        submitInfo.pWaitDstStageMask = &waitStages;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &finishedRenderSem[imageIndex];
        ctx.queue.submit(submitInfo, inFlightFence[frame]);

        vk::PresentInfoKHR presentInfo{};
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &finishedRenderSem[imageIndex];
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &pEngine.swapchain;
        presentInfo.pResults = nullptr;
        result = ctx.queue.presentKHR(presentInfo);

       frame = (frame + 1) % frameInFlight;
    }
}
