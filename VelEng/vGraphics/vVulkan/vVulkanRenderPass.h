#pragma once

#include <vector>

#include "vVulkanCommon.h"
#include "vGeo/vGeo.h"
#include "vVulkanImage.h"

namespace Vel
{

	class Framebuffer : public NonCopyable
	{
	public:
		Framebuffer( VkRenderPass renderPass, const std::vector<VulkanImage> &images, glm::u32vec2 size );
		void Cleanup();
		VkFramebuffer framebuffer;
	};

	class FrameContext
	{
	public:
		FrameContext( VkRenderPass renderPass, const std::vector<VulkanImage> &images, glm::u32vec2 size );
		void Cleanup();
		VkFramebuffer GetFramebuffer() { return swapchainFramebuffer.framebuffer; }
		VkCommandBuffer GetCommandBuffer() { return commandBuffer; }
	private:
		Framebuffer swapchainFramebuffer;
		VkCommandPool commandPool;
		VkCommandBuffer commandBuffer;
		Semaphores semaphores;
	};

    class RenderPass
    {
    public:
		void Create();
		void Cleanup();
		void CreateFrameContexts( const std::vector<VulkanImage> &images, VulkanImage &depthBuffer, glm::i32vec2 size );
		FrameContext& GetFrameContext( uint32_t swapchainImageId );
		const VkRenderPass GetRenderPass() const { return renderPass; }

	private:
		VkRenderPass renderPass;
		std::vector<FrameContext> frameContexts;
    };
}