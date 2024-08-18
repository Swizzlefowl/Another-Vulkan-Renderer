#include "engineUtils.h"
#include <fstream>
namespace avr {
    vk::ImageView createImageView(Context& ctx, vk::Image image, vk::Format format, vk::ImageSubresourceRange range){
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

    vk::CommandPool createCommandPool(Context& ctx){
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

    vk::CommandBuffer createCommandBuffer(Context& ctx, vk::CommandPool& pool){
        vk::CommandBufferAllocateInfo allocateInfo{};
        allocateInfo.commandPool = pool;
        allocateInfo.level = vk::CommandBufferLevel::ePrimary;
        allocateInfo.commandBufferCount = 1;
        return ctx.device.allocateCommandBuffers(allocateInfo)[0];
    }
    std::vector<vk::CommandBuffer> createCommandBuffer(Context& ctx, vk::CommandPool& pool, uint32_t count){
        vk::CommandBufferAllocateInfo allocateInfo{};
        allocateInfo.commandPool = pool;
        allocateInfo.level = vk::CommandBufferLevel::ePrimary;
        allocateInfo.commandBufferCount = count;
        return ctx.device.allocateCommandBuffers(allocateInfo);
    }
    vk::ShaderModule createShader(Context& ctx, const std::string& fileName){
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
    }
    vk::Pipeline createPipeline(Context& ctx, vk::PipelineLayout pipeLayout, vk::DescriptorSetLayout setLayout, const Shaders& shaders, const std::vector<vk::Format> formats){
       
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
        catch (vk::SystemError err) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
        fmt::println("created pipeline");
        ctx.device.destroyShaderModule(shaders.vertShader);
        ctx.device.destroyShaderModule(shaders.fragShader);
        return pipe;
    }
}