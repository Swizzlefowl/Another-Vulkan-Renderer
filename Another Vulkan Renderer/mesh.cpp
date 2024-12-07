#include "mesh.h"
#include <iostream>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

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

        loadTexture(textureName);
        ImageViewBuilder builder{};
        texture.view = builder.setImage(texture.image)
            .setFormat(texture.format)
            .setAspect(vk::ImageAspectFlagBits::eColor)
            .setMips(0, 1)
            .setArrayLayers(0, 1)
            .setViewType(vk::ImageViewType::e2D)
            .createImageView(ctx);
        
        texture.texIndex = ctx.descManager.write(vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eSampledImage, texture.view);
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

    void Mesh::loadTexture(const std::string& name) {
        int texWidth{};
        int texHeight{};
        int texChannels{};
        stbi_uc* pixels{ nullptr };
        pixels = stbi_load(name.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        vk::DeviceSize imageSize{ static_cast<vk::DeviceSize>(texWidth * texHeight * 4) };

        if (!pixels)
            throw std::runtime_error("failed to load image!");
        imageBuilder builder{};
        texture = builder.setWidth(texWidth)
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
            region.imageSubresource.mipLevel = texture.baseMip;
            region.imageSubresource.baseArrayLayer = texture.baseLayer;
            region.imageSubresource.layerCount = texture.layers;

            region.imageOffset = vk::Offset3D{ 0, 0, 0 };
            region.imageExtent = vk::Extent3D{
                texture.width,
                texture.height,
                1 };

            BarrierBuilder builder{};
            auto barrier = builder.setImage(texture.image)
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                .setSubresourceRange(vk::ImageSubresourceRange{
                vk::ImageAspectFlagBits::eColor
                    ,0, 1, 0, 1 })
                .setSrc(vk::PipelineStageFlagBits2::eNone, vk::AccessFlagBits2::eNone)
                .setDst(vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite)
                .createImageBarrier();
            transitionLayout(cb, barrier);
            cb.copyBufferToImage(stagingBuffer, texture.image, vk::ImageLayout::eTransferDstOptimal, region);

            auto barrier2 = builder.setImage(texture.image)
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
        vmaDestroyBuffer(ctx.allocator, stagingBuffer, stagingAlloc);
        stbi_image_free(pixels);
    }
}
