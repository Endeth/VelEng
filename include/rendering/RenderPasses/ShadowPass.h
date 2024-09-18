#pragma once

#include "Rendering/VulkanTypes.h"
#include "Rendering/Descriptors.h"
#include "Rendering/GPUAllocator.h"
#include "Rendering/Lighting.h"
#include "Rendering/Renderable.h"

#include "Rendering/RenderPasses/Pipeline.h"
#include "Rendering/RenderPasses/PipelineBuilder.h"

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
