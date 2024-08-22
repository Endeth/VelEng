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
            AllocatedImage metallicRoughness;
            AllocatedImage depth;

            void TransitionImages(VkCommandBuffer cmd, VkImageLayout src, VkImageLayout dst);
        };

        void Init(VkDevice dev, GPUAllocator* allocator, VkExtent2D renderExtent,
            VkDescriptorSetLayout cameraDescriptorLayout,
            VkBuffer sceneLightDataBuffer, size_t sceneLightDataBufferSize, VkImageView sunlightShadowMapView,
            GPUMeshBuffers&& rect);
        void Cleanup();

        MaterialInstance CreateMaterialInstance(const MaterialResources& resources, DescriptorAllocatorDynamic& descriptorAllocator) const;
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
        VkSampler shadowsSampler;

        VkDescriptorSet sceneCameraDataDescriptorSet;

        GPassPipeline gPass;
        VkDescriptorSetLayout gPassDescriptorLayout;
        AllocatedImage defaultColorMap;
        AllocatedImage defaultNormalMap;
        AllocatedImage defaultMetalRoughnessMap;
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

        VkRenderingAttachmentInfo framebufferAttachments[4];
        VkRenderingAttachmentInfo gPassDepthAttachmentInfo;
        VkRenderingAttachmentInfo lPassDrawAttachment;
        VkRenderingInfo gPassRenderInfo;
        VkRenderingInfo lPassRenderInfo;

        VkViewport renderViewport;
        VkRect2D renderScissor;

        void PreBuildRenderInfo();
        VkRenderingAttachmentInfo BuildGPassAttachmentInfo(VkImageView imageView, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
        VkRenderingAttachmentInfo BuildLPassAttachmentInfo(VkImageView imageView);
        VkRenderingAttachmentInfo BuildDepthAttachmentInfo();
        VkRenderingInfo BuildRenderInfo(VkRenderingAttachmentInfo* color, uint32_t colorAttachmentsCount, VkRenderingAttachmentInfo* depth);
        VkViewport BuildRenderViewport();
        VkRect2D BuildRenderScissors();
    };
}
