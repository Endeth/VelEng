#pragma once

#include <vector>

#include "vVulkanCommon.h"
#include "vGeo/vGeo.h"


namespace Vel
{
	class VulkanFramebuffer
	{
	public:
		VulkanFramebuffer( VkRenderPass renderPass, const std::vector<VulkanImage> &attachements, glm::u32vec2 size );
		void Cleanup();
		VkFramebuffer _framebuffer;
	};

    class VulkanRenderPass
    {
    public:
		VulkanRenderPass() :
			_renderPass( VK_NULL_HANDLE ),
			_graphicsPipeline( VK_NULL_HANDLE )
		{}

		void Create();
		void CreatePipeline();
		void CreateFramebuffers( const std::vector<VulkanImage> &images, glm::i32vec2 size );
		void Cleanup();

		VkRenderPass _renderPass;
		std::vector<VulkanFramebuffer> _framebuffers;
		VkPipeline _graphicsPipeline;
		VkPipelineCache _pipelineCache;
	private:
		VkShaderModule CreateShaderModule( const char *filepath );
    };
}