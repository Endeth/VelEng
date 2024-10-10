#pragma once

#include "Rendering/VulkanTypes.h"

#include "Rendering/Buffers/GPUAllocator.h"

#include "Rendering/RenderPasses/Descriptors.h"
#include "Rendering/RenderPasses/Pipeline.h"
#include "Rendering/RenderPasses/PipelineBuilder.h"

#include "Rendering/Scene/Lighting.h"
#include "Rendering/Scene/Renderable.h"

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
        void Init(VkDevice dev);
        void Draw(const DrawContext& context, VkCommandBuffer cmd, const Sunlight& sunlight);
        void UpdateDescriptorSet(const Sunlight& sunlight);

        void Cleanup();

    private:
        VkDevice device;
        DescriptorAllocatorDynamic descriptorPool;
        DescriptorWriter descriptorWriter;

        VkDescriptorSetLayout shadowPassLayout;
        VkDescriptorSet shadowPassDescriptorSet;
        ShadowPipeline pipeline;

        //VkExtent3D drawExtent;
        //VkImageView drawImageView;
        //VkImage drawImage;

        VkRenderingAttachmentInfo depthAttachment;
        VkRenderingInfo renderingInfo;
        VkViewport renderViewport;
        VkRect2D renderScissor;

        void PrebuildRenderInfo();
    };
}
