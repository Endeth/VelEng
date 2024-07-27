#pragma once
#include "VulkanTypes.h"
#include "GPUAllocator.h"
#include "Descriptors.h"
#include "Pipeline.h"

namespace Vel
{
    class GLTFMetallicRoughness
    {
    public:
        struct MaterialConstants
        {
            glm::vec4 color;
            glm::vec4 metallicFactor;

            //padding and uniform buffers
            glm::vec4 extra[14];
        };

        struct MaterialResources
        {
            AllocatedImage colorImage;
            VkSampler colorSampler;
            AllocatedImage metallicImage;
            VkSampler metallicSampler;
            VkBuffer dataBuffer;
            uint32_t dataBufferOffset;
        };

        void Init(VkDevice dev, VkDescriptorSetLayout sceneDescriptorsLayout, VkFormat drawImgFormat, VkFormat depthImgFormat);
        void BuildPipelines();
        void ClearResources();

        MaterialInstance WriteMaterial(MaterialPass pass, const MaterialResources& resources, DescriptorAllocatorDynamic& descriptorAllocator);

    private:
        VkDevice device;

        MaterialPipeline opaquePipeline;
        MaterialPipeline transparentPipeline;
        VkFormat drawImageFormat;
        VkFormat depthImageFormat;

        VkDescriptorSetLayout materialLayout;
        VkDescriptorSetLayout sceneLayout;
        DescriptorWriter writer;

    };
}
