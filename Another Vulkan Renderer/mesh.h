#pragma once
#include "context.h"
#include "engineUtils.h"
#include <vector>
namespace avr {
    class Mesh {
    public:
        Mesh(Context& otherCtx);
        void createMesh(const std::string& modelName, const std::string& textureName);
        vk::Buffer vertBuffer{};
        vk::DeviceAddress vertexAdress{};
        VmaAllocation vertAlloc{};
        vk::Buffer indexBuffer{}; //unused for now
        VmaAllocation indexAlloc{};
        std::uint32_t vertCount{};
        std::uint32_t indexCount{};
        vk::Image texture{}; //unused for now
        VmaAllocation texAlloc{};
    private:
        Context& ctx;
        void loadModel(const std::string& name, std::vector<Vertex>& vertices, std::vector<std::uint32_t>& indices);
        void loadTexture(const std::string& name);
    };
}

