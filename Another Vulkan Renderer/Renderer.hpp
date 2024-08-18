#pragma once
#include "context.h"
#include "presentEngine.hpp"
namespace avr {
    class Renderer {
    public:
        void init();
        void init(size_t height, size_t width, const std::string& title);
        Renderer();
        ~Renderer();
        void preparePipeline(vk::Pipeline& pipe);
        void createSyncObjects();
        void recordCB(vk::CommandBuffer& cBuffer, uint32_t imageIndex);
        void mainLoop();
        void drawFrame();
        Context ctx{};
        PresentEngine pEngine{ctx};
        vk::PipelineLayout pipeLayout{};
        vk::Pipeline graphicsPipe{};
        vk::Semaphore aquireSem{};
        vk::Semaphore finishedRenderSem{};
        vk::Fence inFlightFence{};
        DeletionQueue renderDelQueue{};
    };
}

