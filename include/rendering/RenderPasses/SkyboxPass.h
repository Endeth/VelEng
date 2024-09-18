#pragma once

#include "Rendering/VulkanTypes.h"
#include "Rendering/Descriptors.h"
#include "Rendering/GPUAllocator.h"
#include "Rendering/Camera.h"

#include "Rendering/RenderPasses/Pipeline.h"
#include "Rendering/RenderPasses/PipelineBuilder.h"

namespace Vel
{
    class SkyboxPipeline : public VulkanPipeline
    {
    public:
        VkPipelineBuilder pipelineBuilder;
        void CreatePipeline(VkDescriptorSetLayout* layouts, uint32_t layoutsCount);
    };

    class SkyboxPass
    {
    public:
        void Init(VkDevice dev, VkExtent2D renderExtent, AllocatedImage& skyboxImage, AllocatedImage& drawImage);
        void Draw(VkCommandBuffer cmd, const Camera &camera);

        void Cleanup();

    private:
        VkDevice device;
        DescriptorAllocatorDynamic descriptorPool;
        DescriptorWriter descriptorWriter;

        VkDescriptorSetLayout skyboxLayout;
        VkDescriptorSet skyboxDescriptorSet;
        SkyboxPipeline pipeline;

        VkExtent2D drawExtent;
        VkSampler sampler;
        VkImageView drawImageView;

        VkRenderingAttachmentInfo renderingAttachmentInfo;
        VkRenderingInfo renderingInfo;
        VkViewport renderViewport;
        VkRect2D renderScissor;

        void PrebuildRenderInfo();
        VkRenderingAttachmentInfo BuildAttachmentInfo();
        VkRenderingInfo BuildRenderInfo();
        VkViewport BuildRenderViewport();
        VkRect2D BuildRenderScissors();
    };
}
