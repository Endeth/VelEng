#pragma once

#include "vVulkanCommon.h"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif

namespace Vel
{
	class ShaderDescription
	{
	public:
		ShaderDescription( VkShaderModule vertex, VkShaderModule fragment );
		~ShaderDescription();
		VkShaderModule CreateShaderModule( const char *filepath );
		bool DestroyShaderModules();

		VkShaderModule vertexShader;
		VkShaderModule fragmentShader;

		VkPipelineShaderStageCreateInfo shaderStageInfo[2];
		VkVertexInputBindingDescription vertexInputBindingDescription;
		VkVertexInputAttributeDescription vertexInputAttributeDescription[3];
		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo;
		VkDescriptorSetLayout dscSetLayout;
	};

	class ShaderInstance
	{
	public:
		std::shared_ptr<ShaderDescription> shaderDescription;
	};
}