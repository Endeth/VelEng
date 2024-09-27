#pragma once
#include <deque>

#include "Rendering/VulkanTypes.h"

#include "Rendering/Buffers/Buffers.h"
#include "Rendering/Buffers/GPUAllocator.h"

#include "Rendering/RenderPasses/Descriptors.h"
#include "Rendering/RenderPasses/Pipeline.h"
#include "Rendering/RenderPasses/PipelineBuilder.h"

#include "Rendering/Scene/Renderable.h"


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

    struct DeferredMaterialResources
    {
        AllocatableImage colorImage;
        VkSampler colorSampler; //Use one sampler;
        AllocatableImage normalsImage;
        VkSampler normalsSampler;
        AllocatableImage metallicRoughnessImage;
        VkSampler metallicRoughnessSampler;
        AllocatableImage emissiveImage; //separate?
        VkSampler emissiveSampler;
        VkBuffer dataBuffer; //Model materials constants buffer
        uint32_t dataBufferOffset; //Material offset
    };

    struct DrawContext;
    //TODO Separate
    class DeferredPasses
    {
    public:
        struct Framebuffer
        {
            static constexpr uint8_t FRAMEBUFFER_SIZE = 4;

            enum ImageType : uint8_t
            {
                POSITION = 0,
                COLOR = 1,
                NORMALS = 2,
                METALLIC_ROUGHNESS = 3,
            };

            AllocatableImage position;
            AllocatableImage color;
            AllocatableImage normals;
            AllocatableImage metallicRoughness;

            AllocatableImage depth;

            VkDescriptorSet framebufferDescriptor;

            using Dependencies = std::array<VkImageMemoryBarrier2, Vel::DeferredPasses::Framebuffer::FRAMEBUFFER_SIZE>;
            Dependencies GetPipelineBarrierDependencyInfo(const ImageBarrierInfo& src, const ImageBarrierInfo& dst);
            void TransitionImages(VkCommandBuffer cmd, VkImageLayout src, VkImageLayout dst);
            void TransitionImagesForAttachment(VkCommandBuffer cmd);
            void TransitionImagesForDescriptors(VkCommandBuffer cmd);
        };

        void Init(VkDevice dev, VkDescriptorSetLayout cameraDescriptorLayout);
        void Cleanup();

        MaterialInstance CreateMaterialInstance(const DeferredMaterialResources& resources, DescriptorAllocatorDynamic& descriptorAllocator) const;
        Framebuffer CreateUnallocatedFramebuffer(const VkExtent3D& extent);
        AllocatableImage CreateUnallocatedLPassDrawImage(const VkExtent3D& extent);

        VkDescriptorSetLayout GetLightDescriptorSetLayout() { return lightsDescriptorLayout; }
        void UpdateFramebufferDescriptor(Framebuffer& framebuffer) const;

        void SetRenderExtent(const VkExtent2D& extent);

        void DrawGPass(const std::vector<DrawContext>& contexts, VkCommandBuffer cmd, const Framebuffer& framebuffer, const VkDescriptorSet& scene);
        void DrawLPass(VkCommandBuffer cmd, const AllocatableImage& drawImage, const VkDescriptorSet& scene, const VkDescriptorSet& framebufferDescriptor, const VkDescriptorSet& lightsDescriptor);

    private:
        VkDevice device;
        DescriptorAllocatorDynamic descriptorPool;

        VkSampler sampler;
        VkSampler shadowsSampler;

        GPassPipeline gPass;
        VkRenderingInfo gPassRenderInfo;
        VkDescriptorSetLayout gPassDescriptorLayout;
        VkRenderingAttachmentInfo gPassDepthAttachmentInfo;
        //TODO until asset manager
        //AllocatedImage defaultColorMap;
        //AllocatedImage defaultNormalMap;
        //AllocatedImage defaultMetalRoughnessMap;

        LPassPipeline lPass;
        VkRenderingInfo lPassRenderInfo;
        VkDescriptorSetLayout framebufferDescriptorLayout;
        VkDescriptorSetLayout lightsDescriptorLayout;
        //VkDescriptorSet framebufferDescriptorSet;
        VkDescriptorSet lightsDescriptorSet;
        VkRenderingAttachmentInfo lPassDrawAttachment;
        //AllocatedImage drawImage;

        VkRenderingAttachmentInfo framebufferAttachments[4];

        VkViewport renderViewport;
        VkRect2D renderScissor;

        void CreateSamplers();
        void CreateGPassDescriptorLayouts();
        void CreateLPassDescriptorLayouts();

        void SetFramebufferGPassAttachment(const Framebuffer& framebuffer);

        void BuildRenderInfo();
        VkRenderingAttachmentInfo BuildAttachmentInfo(VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
        VkRenderingAttachmentInfo BuildDepthAttachmentInfo();
        VkRenderingInfo BuildRenderInfo(VkRenderingAttachmentInfo* color, uint32_t colorAttachmentsCount, VkRenderingAttachmentInfo* depth);
        VkViewport BuildRenderViewport();
        VkRect2D BuildRenderScissors();
    };
}
