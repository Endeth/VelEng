#pragma once

#include <vector>

#include "vGeo/vGeo.h"
#include "vVulkanCommon.h"
#include "vVulkanImage.h"
#include "vVulkanShader.h"

namespace Vel
{
	class Pipeline
	{
	public:
		void Create( VkRenderPass renderPass, VulkanShader shader );
		void Cleanup();

		const VkPipeline Get() const { return pipeline; }
	private:
		void CreatePipelineLayout( VkDescriptorSetLayout dscSetLayout );
		void CreatePipeline( VkRenderPass renderPass, VkShaderModule vertex, VkShaderModule fragment );

		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		VkShaderModule CreateShaderModule( const char *filepath );

		//static void CreatePipelines( std::vector<Pipeline> &pipelines );
	};
	
	//class VulkanGraphicsPipeline : public Pipeline {}; TODO
}