#pragma once
#include "context.h"
#include "presentEngine.hpp"
#include "engineUtils.h"
#include "mesh.h"
#include "Image.hpp"
#include "VideoPlayer.hpp"
namespace avr {
    class Renderer {
    public:
        void init();
        void init(size_t height, size_t width, const std::string& title);
        Renderer();
        ~Renderer();
        void createSampler();
        void preparePipeline(vk::Pipeline& pipe);
        void createSyncObjects();
        void createVertexBuffer();
        void createDepthBuffer();
        void registerMeshes();
        void createDisplayImage();
        void displayVideo(vk::CommandBuffer& cBuffer, uint32_t imageIndex);
        void recordCB(vk::CommandBuffer& cBuffer, uint32_t imageIndex);
        void mainLoop();
        void drawFrame();
        const uint32_t frameInFlight{2};
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
        Image loc{};
        Image disImage{};
        Sampler sampler{};
        VideoPlayer player{ "teaser.mp4", ctx};
        vk::Buffer stagingBuffer{ nullptr };
        VmaAllocation stagingAlloc{};
        void* mappedPtr{ nullptr };
        std::vector<uint8_t> renderedData{};
        DeletionQueue renderDelQueue{};
    };
}

