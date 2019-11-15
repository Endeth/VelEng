#pragma once

#include <vector>

#include "vVulkanCommon.h"
#include "vVulkanImage.h"
#include "vVulkanShader.h"

namespace Vel
{
	class Pipeline
	{
	public:
		void Create( VkRenderPass renderPass, ShaderDescription shader );
		void Cleanup();

		const VkPipeline Get() const { return pipeline; }
	private:
		void CreatePipeline( VkRenderPass renderPass, ShaderDescription shader );

		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		VkShaderModule CreateShaderModule( const char *filepath );

		//static void CreatePipelines( std::vector<Pipeline> &pipelines );
	};
	
	//class VulkanGraphicsPipeline : public Pipeline {}; TODO
}