#pragma once
#include "VulkanTypes.h"
#include "Descriptors.h"
#include "Pipeline.h"
#include "PipelineBuilder.h"
#include "GPUAllocator.h"
#include "Renderable.h"
#include <deque>

namespace Vel
{
    class GPassPipeline : public VulkanPipeline
    {
    public:
        VkPipelineBuilder pipelineBuilder;
        void CreatePipeline(VkDescriptorSetLayout* layouts, uint32_t layoutsCount);
    };

    class LPassPipeline : public VulkanPipeline
    {
    public:
        VkPipelineBuilder pipelineBuilder;
        void CreatePipeline(VkDescriptorSetLayout* layouts, uint32_t layoutsCount);
    };

    class DeferredRenderer
    {
    public:
        struct Framebuffer
        {
            AllocatedImage position;
            AllocatedImage color;
            AllocatedImage normals;
            AllocatedImage depth;
        };

        void Init(VkDevice dev, GPUAllocator* allocator, VkExtent2D renderExtent,
            VkDescriptorSetLayout cameraDescriptorLayout,
            VkBuffer sceneLightDataBuffer, size_t sceneLightDataBufferSize,
            GPUMeshBuffers&& rect);
        void Cleanup();

        void UpdateCameraDescriptorSet(VkDescriptorSet cameraSet) { sceneCameraDataDescriptorSet = cameraSet; }

        void DrawGPass(const DrawContext& context, VkCommandBuffer cmd);
        //void DrawShadows(const DrawContext& context);
        void DrawLPass(const DrawContext& context, VkCommandBuffer cmd);

        VkSemaphore& GetGPassSemaphore() { return gPassFinishDrawing; }
        VkSemaphore& GetLPassSemaphore() { return lPassFinishDrawing; }

        Framebuffer& GetFramebuffer() { return framebuffer; }
        AllocatedImage& GetDrawImage() { return drawImage; }

    private:
        VkDevice device;
        GPUAllocator* mainAllocator;
        DescriptorAllocatorDynamic descriptorPool;
        DescriptorWriter descriptorWriter;

        VkExtent2D drawExtent;
        VkSampler sampler;

        VkDescriptorSet sceneCameraDataDescriptorSet;

        GPassPipeline gPass;
        VkDescriptorSetLayout gPassDescriptorLayout;
        AllocatedImage defaultColorMap;
        AllocatedImage defaultNormalMap;
        AllocatedImage defaultSpecularMap;
        Framebuffer framebuffer;
        VkSemaphore gPassFinishDrawing;

        LPassPipeline lPass;
        VkDescriptorSetLayout framebufferDescriptorLayout;
        VkDescriptorSetLayout lightsDescriptorLayout;
        VkDescriptorSet framebufferDescriptorSet;
        VkDescriptorSet lightsDescriptorSet;
        GPUMeshBuffers drawRect;
        AllocatedImage drawImage;
        VkSemaphore lPassFinishDrawing;

        //Testing
        VkDescriptorSet testGPassSet;

        //gpass prebuilt render info
        VkRenderingAttachmentInfo framebufferAttachments[3];
        VkRenderingAttachmentInfo gPassDepthAttachmentInfo;
        VkRenderingAttachmentInfo lPassDrawAttachment;
        VkRenderingInfo gPassRenderInfo;
        VkRenderingInfo lPassRenderInfo;

        VkViewport renderViewport;
        VkRect2D renderScissor;

        void PreBuildRenderInfo();
        VkRenderingAttachmentInfo BuildGPassAttachmentInfo(VkImageView imageView);
        VkRenderingAttachmentInfo BuildLPassAttachmentInfo(VkImageView imageView);
        VkRenderingAttachmentInfo BuildDepthAttachmentInfo();
        VkRenderingInfo BuildRenderInfo(VkRenderingAttachmentInfo* color, uint32_t colorAttachmentsCount, VkRenderingAttachmentInfo* depth);
        VkViewport BuildRenderViewport();
        VkRect2D BuildRenderScissors();
    };
}
