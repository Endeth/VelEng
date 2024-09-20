#pragma once

#include "Rendering/VulkanTypes.h"

#include "Rendering/Buffers/GPUAllocator.h"

#include "Rendering/RenderPasses/Descriptors.h"
#include "Rendering/RenderPasses/Pipeline.h"
#include "Rendering/RenderPasses/PipelineBuilder.h"

#include "Rendering/Scene/Camera.h"


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
        void Init(VkDevice dev, AllocatedImage& skyboxImage);
        void Draw(VkCommandBuffer cmd, const Camera &camera, const AllocatedImage& drawImage);

        void Cleanup();

    private:
        VkDevice device;
        DescriptorAllocatorDynamic descriptorPool;
        DescriptorWriter descriptorWriter;

        VkDescriptorSetLayout skyboxLayout;
        VkDescriptorSet skyboxDescriptorSet;
        SkyboxPipeline pipeline;

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
