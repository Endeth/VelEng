#pragma once
#include "VulkanTypes.h"
#include "Descriptors.h"
#include "Pipeline.h"
#include "PipelineBuilder.h"
#include "GPUAllocator.h"
#include "Lighting.h"
#include "Renderable.h"

namespace Vel
{
    class ShadowPipeline : public VulkanPipeline
    {
    public:
        VkPipelineBuilder pipelineBuilder;
        void CreatePipeline(VkDescriptorSetLayout* layouts, uint32_t layoutsCount);
    };

    class ShadowPass
    {
    public:
        void Init(VkDevice dev, Sunlight& sunlight);
        void Draw(const DrawContext& context, VkCommandBuffer cmd);

        void Cleanup();

    private:
        VkDevice device;
        DescriptorAllocatorDynamic descriptorPool;
        DescriptorWriter descriptorWriter;

        VkDescriptorSetLayout shadowPassLayout;
        VkDescriptorSet shadowPassDescriptorSet;
        ShadowPipeline pipeline;

        VkExtent3D drawExtent;
        VkImageView drawImageView;
        VkImage drawImage;

        VkRenderingAttachmentInfo depthAttachment;
        VkRenderingInfo renderingInfo;
        VkViewport renderViewport;
        VkRect2D renderScissor;

        void PrebuildRenderInfo();
    };
}
