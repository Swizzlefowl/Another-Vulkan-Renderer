#include "mesh.h"
#include <iostream>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
namespace avr {
    Mesh::Mesh(Context& otherCtx) : ctx{ otherCtx } {

    }

    void Mesh::createMesh(const std::string& modelName, const std::string& textureName) {
        std::vector<Vertex> vertices{};
        std::vector<std::uint32_t> indices{};
        loadModel(modelName, vertices, indices);
        vk::DeviceSize size{ sizeof(vertices[0]) * vertices.size() };
        VmaAllocationCreateInfo info{};
        info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        vertBuffer = createBuffer(ctx, info, vk::BufferUsageFlagBits::eShaderDeviceAddress, size, vertAlloc);
        mapMemory(ctx, vertAlloc, vertices.data(), size);

        vk::BufferDeviceAddressInfo addrInfo{ vertBuffer };
        vertexAdress = ctx.device.getBufferAddress(addrInfo);
        vertCount = vertices.size();
    }

    void Mesh::loadModel(const std::string& name, std::vector<Vertex>& vertices, std::vector<std::uint32_t>& indices) {
        Assimp::Importer importer{};
        const aiScene* scene{ nullptr };
        scene = importer.ReadFile(name.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);
        // | aiProcess_JoinIdenticalVertices

        if (!scene)
            throw std::runtime_error("you done fucked up");
        std::cout << scene->mNumMeshes;
        auto mesh = scene->mMeshes[0];
        vertices.resize(scene->mMeshes[0]->mNumVertices);

        for (size_t index{}; index < scene->mMeshes[0]->mNumVertices; index++) {
            Vertex vertice{};

            vertice.pos.r = mesh->mVertices[index].x;
            vertice.pos.g = mesh->mVertices[index].y;
            vertice.pos.b = mesh->mVertices[index].z;

            vertice.color.r = 1.0;
            vertice.color.g = 0.0;
            vertice.color.b = 0.0;

            vertice.texCoord.r = mesh->mTextureCoords[0][index].x;
            vertice.texCoord.g = mesh->mTextureCoords[0][index].y;
            vertices[index] = vertice;
        }
    }

    void Mesh::loadTexture(const std::string& name){
    }
}
