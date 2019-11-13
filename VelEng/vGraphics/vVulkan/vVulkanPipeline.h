#pragma once

#include <vector>

#include "vVulkanCommon.h"
#include "vGeo/vGeo.h"
#include "vVulkanImage.h"

namespace Vel
{
	class Pipeline
	{
	public:
		void Create( VkRenderPass renderPass, VkDescriptorSetLayout dscSetLayout );
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