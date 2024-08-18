#include "Renderer.hpp"
#include "engineUtils.h"
namespace avr {
    void Renderer::init() {
        ctx.createWindow(720, 640, "default name");
        ctx.initVulkanCtx();
        pEngine.createSwapchain();
        pEngine.createSwapchainImageViews();
        preparePipeline(graphicsPipe);
        createSyncObjects();
    }

    void Renderer::init(size_t height, size_t width, const std::string& title) {
        ctx.createWindow(height, width, title);
        ctx.initVulkanCtx();
        pEngine.createSwapchain();
        pEngine.createSwapchainImageViews();
        preparePipeline(graphicsPipe);
        createSyncObjects();
    }

    Renderer::Renderer() {
    }

    Renderer::~Renderer() {
    }

    void Renderer::preparePipeline(vk::Pipeline& pipe) {
        std::vector<vk::Format> formats{ pEngine.swapChainImagesFormat };
        Shaders shaders{};
        shaders.type = avr::pipeLineType::Graphics;
        shaders.vertShader = avr::createShader(ctx, "vertex.spv");
        shaders.fragShader = avr::createShader(ctx, "fragment.spv");

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

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
    void Renderer::createSyncObjects(){
        inFlightFence = avr::createVKFence(ctx);
        aquireSem = avr::createVKSemaphore(ctx);
        finishedRenderSem = avr::createVKSemaphore(ctx);
        renderDelQueue.enqueue([&]() {
            ctx.device.waitIdle();
            ctx.device.destroySemaphore(aquireSem);
            ctx.device.destroySemaphore(finishedRenderSem);
            ctx.device.destroyFence(inFlightFence);
            fmt::println("destroyed semaphore and fence");
            }
        );
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

        rInfo.colorAttachmentCount = 1;
        rInfo.pColorAttachments = &aInfo;
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

        transitionLayout(cBuffer, barrier);
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
        cBuffer.draw(3, 1, 0, 0);
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
            //changeColor(checkUserInput());
            drawFrame();
        }
        ctx.device.waitIdle();
    }
    void Renderer::drawFrame(){
        ctx.device.waitForFences(inFlightFence, VK_TRUE, UINT64_MAX);
        vk::Result result;
        uint32_t imageIndex{};

        // unfortunately vk raii seems to throw an exception only on Nvidia if you cant present
        //  or aquire images anymore because the surface is incompatible
        //  so the try catch blocks are necessary to successfully recreate
        //  the swapchain
            std::tie(result, imageIndex) = ctx.device.acquireNextImageKHR(pEngine.swapchain,
                UINT64_MAX, aquireSem
            );
        
        ctx.device.resetFences(inFlightFence);

        ctx.commandBuffer.reset();
        recordCB(ctx.commandBuffer, imageIndex);
        //recordComputeCB(pResources->commandBuffer[0], imageIndex);
        vk::SubmitInfo submitInfo{};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &aquireSem;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &ctx.commandBuffer;
        vk::PipelineStageFlags waitStages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
        submitInfo.pWaitDstStageMask = &waitStages;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &finishedRenderSem;
        ctx.queue.submit(submitInfo, inFlightFence);

        vk::PresentInfoKHR presentInfo{};
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &finishedRenderSem;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &pEngine.swapchain;
        presentInfo.pResults = nullptr;
        result = ctx.queue.presentKHR(presentInfo);
    }
}
