#pragma once

#include "Rendering/VulkanTypes.h"

namespace Vel
{
    class VkPipelineBuilder
    {
    public:
        void Init(VkDevice dev);
        void Reset();

        void SetPipelineLayout(VkPipelineLayout layout);
        void SetPipelineLayoutGPass(VkPipelineLayout layout, uint32_t colAttachmentsCount);
        void SetShaders(VkShaderModule vertex, VkShaderModule fragment);
        void SetInputTopology(VkPrimitiveTopology topology);
        void SetPolygonMode(VkPolygonMode mode);
        void SetCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);
        void SetMultisampling();
        void EnableBlendingAdditive();
        void EnableBlendingAlphablend();
        void DisableBlending();
        void SetColorAttachmentFormat(VkFormat format);
        void SetColorAttachmentsFormats(VkFormat* formats, uint32_t formatsCount);
        void SetDepthFormat(VkFormat format);
        void EnableDepthTest(bool depthWriteEnable, VkCompareOp op);
        void DisableDepthTest();

        VkPipeline BuildGfxPipeline(VkDevice device);
    private:
        VkDevice device;

        VkPipelineLayout pipelineLayout;
        uint32_t colorAttachmentsCount;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        VkFormat colorAttachmentFormat;
        VkPipelineInputAssemblyStateCreateInfo inputAssembly;
        VkPipelineRasterizationStateCreateInfo rasterizer;
        VkPipelineColorBlendAttachmentState colorBlendAttachment[4];
        VkPipelineMultisampleStateCreateInfo multisampling;
        VkPipelineDepthStencilStateCreateInfo depthStencil;
        VkPipelineRenderingCreateInfo renderInfo;
    };
}
