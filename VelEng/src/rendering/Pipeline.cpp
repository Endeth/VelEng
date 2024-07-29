#include "Pipeline.h"
#include "PipelineBuilder.h"

#include <fstream>

Vel::VkPipelineBuilder pipelineBuilder{};

//Shader

bool Vel::LoadShaderModule(const char* spvPath, VkDevice device, VkShaderModule* shaderModule)
{
    std::ifstream file(spvPath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
    file.seekg(0);
    file.read((char*)buffer.data(), fileSize);
    file.close();

    VkShaderModuleCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .codeSize = buffer.size() * sizeof(uint32_t),
        .pCode = buffer.data()
    };

    if (vkCreateShaderModule(device, &createInfo, nullptr, shaderModule) != VK_SUCCESS)
    {
        return false;
    }

    return true;
}

//Pipelines

void Vel::VulkanPipeline::SetDevice(VkDevice dev)
{
    device = dev;
}

void Vel::VulkanPipeline::Cleanup()
{
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
}

const VkPipeline& Vel::VulkanPipeline::GetPipeline()
{
    return pipeline;
}

const VkPipelineLayout& Vel::VulkanPipeline::GetPipelineLayout()
{
    return pipelineLayout;
}

void Vel::VulkanComputePipeline::CreatePipeline(VkDescriptorSetLayout* layouts, uint32_t layoutsCount)
{
    VkPipelineLayoutCreateInfo computeLayoutInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .setLayoutCount = layoutsCount,
        .pSetLayouts = layouts,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };

    VK_CHECK(vkCreatePipelineLayout(device, &computeLayoutInfo, nullptr, &pipelineLayout));

    VkShaderModule computeDrawShader;
    if (!LoadShaderModule(GET_SHADER_PATH("gradient_push.comp"), device, &computeDrawShader))
    {
        fmt::print("Error when building the compute shader \n");
    }

    VkPipelineShaderStageCreateInfo stageInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = computeDrawShader,
        .pName = "main"
    };

    VkComputePipelineCreateInfo pipelineCreateInfo {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .stage = stageInfo,
        .layout = pipelineLayout
    };

    VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline));

    vkDestroyShaderModule(device, computeDrawShader, nullptr);
}
