#pragma once
#include "VulkanTypes.h"
#include <deque>

#define GET_SHADER_PATH(name) SHADERS_PATH name ".spv"
namespace Vel
{
    bool LoadShaderModule(const char* spvPath, VkDevice device, VkShaderModule* shaderModule);

    class VulkanPipeline
    {
    public:
        void SetDevice(VkDevice dev);
        //void CreatePipeline(VkDescriptorSetLayout* layouts, uint32_t layoutsCount) {};
        void Cleanup();
        const VkPipeline& GetPipeline();
        const VkPipelineLayout& GetPipelineLayout();
    protected:
        VkDevice device;
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
    };

    class VulkanComputePipeline : public VulkanPipeline
    {
    public:
        void CreatePipeline(VkDescriptorSetLayout* layouts, uint32_t layoutsCount);
    };
}
