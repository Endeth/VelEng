#pragma once

#include "vVulkanCommon.h"
#include "vVulkanImage.h"

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
		void DestroyShaderModules();

		VkShaderModule vertexShader;
		VkShaderModule fragmentShader;

		VkPipelineShaderStageCreateInfo shaderStageInfo[2];
		VkVertexInputBindingDescription vertexInputBindingDescription;
		VkVertexInputAttributeDescription vertexInputAttributeDescription[3];
		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo;
		VkDescriptorSetLayout dscSetLayout;
		VkDescriptorPool descriptorPool;

		int32_t bindings = 2;
	};

	class DecriptorSet
	{

	};

	class ShaderInstance
	{
	public:
		ShaderInstance( std::shared_ptr<ShaderDescription> shaderDesc );
		bool UpdateDescriptorSets();
		std::shared_ptr<ShaderDescription> shaderDescription;
		VkImageView sampledImage;
	};
}