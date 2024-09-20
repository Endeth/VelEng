#pragma once

#include "Rendering/VulkanTypes.h"

#include "Rendering/Buffers/GPUAllocator.h"

#include "Rendering/RenderPasses/Descriptors.h"
#include "Rendering/RenderPasses/DeferredPasses.h"
#include "Rendering/RenderPasses/Pipeline.h"

namespace Vel
{
    struct DeferredMaterialResources;
    class GLTFMetallicRoughness
    {
    public:
        void Init(VkDevice dev, VkDescriptorSetLayout sceneDescriptorsLayout, VkFormat drawImgFormat, VkFormat depthImgFormat);
        void BuildPipelines();
        void ClearResources();

        MaterialInstance WriteMaterial(MaterialPass pass, const DeferredMaterialResources& resources, DescriptorAllocatorDynamic& descriptorAllocator);

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
