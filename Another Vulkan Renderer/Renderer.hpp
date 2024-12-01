#pragma once
#include "context.h"
#include "presentEngine.hpp"
#include "engineUtils.h"
#include "mesh.h"
#include "Image.hpp"
namespace avr {
    class Renderer {
    public:
        void init();
        void init(size_t height, size_t width, const std::string& title);
        Renderer();
        ~Renderer();
        void createGlobalSet();
        void createSampler();
        void preparePipeline(vk::Pipeline& pipe);
        void createSyncObjects();
        void createVertexBuffer();
        void createDepthBuffer();
        void registerMeshes();
        void recordCB(vk::CommandBuffer& cBuffer, uint32_t imageIndex);
        void mainLoop();
        void drawFrame();
        const uint32_t frameInFlight{ 2 };
        uint32_t frame{};
        Context ctx{};
        PresentEngine pEngine{ctx};
        vk::PipelineLayout pipeLayout{};
        vk::Pipeline graphicsPipe{};
        avr::Image depthImage{};
        std::vector<vk::Semaphore> aquireSem{2};
        std::vector<vk::Semaphore> finishedRenderSem{};
        std::vector<vk::Fence> inFlightFence{2};
        Mesh mesh{ ctx };
        vk::Sampler sampler{};
        DeletionQueue renderDelQueue{};
    };
}

