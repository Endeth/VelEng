#pragma once
#include "VulkanTypes.h"
#include "Pipeline.h"
#include "PipelineBuilder.h"
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
}
