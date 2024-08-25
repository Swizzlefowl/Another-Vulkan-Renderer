#pragma once
#include "context.h"
#include "presentEngine.hpp"
#include "engineUtils.h"
#include "mesh.h"
namespace avr {
    class Renderer {
    public:
        void init();
        void init(size_t height, size_t width, const std::string& title);
        Renderer();
        ~Renderer();
        void preparePipeline(vk::Pipeline& pipe);
        void createSyncObjects();
        void createVertexBuffer();
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
        std::vector<vk::Semaphore> aquireSem{2};
        std::vector<vk::Semaphore> finishedRenderSem{};
        std::vector<vk::Fence> inFlightFence{2};
        vk::Buffer vertexBuffer{ nullptr };
        VmaAllocation vertexAlloc{nullptr};
        vk::DeviceAddress vertexAdress{};
        Mesh mesh{ ctx };
        DeletionQueue renderDelQueue{};
    };
}

