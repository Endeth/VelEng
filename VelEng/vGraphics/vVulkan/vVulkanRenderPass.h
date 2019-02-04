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
		void Create();
		void CreatePipeline( VkDescriptorSetLayout dscSetLayout, VkPipelineLayout &_pipelineLayout );
		void CreateFramebuffers( const std::vector<VulkanImage> &images, glm::i32vec2 size );
		void Cleanup();

		VkRenderPass _renderPass;
		std::vector<VulkanFramebuffer> _framebuffers;
		VkPipeline _graphicsPipeline = VK_NULL_HANDLE;
		VkPipelineCache _pipelineCache = VK_NULL_HANDLE;
		VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;
	private:
		VkShaderModule CreateShaderModule( const char *filepath );
    };
}