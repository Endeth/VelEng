#pragma once

#include <vector>

#include "vGeo/vGeo.h"
#include "vVulkanCommon.h"
#include "vVulkanImage.h"
#include "vVulkanPipeline.h"

namespace Vel
{
	/*TODO what swapchain resources does framebuffer needs to keep?*/
	class Framebuffer : public NonCopyable
	{
	public:
		Framebuffer( VkRenderPass renderPass, const std::vector<VulkanImage> &images, glm::u32vec2 size );
		void Cleanup();

		VkFramebuffer framebuffer;
		std::vector<VkImageView> attachements;
		VkImage swapchainImage;
	};

	class FrameContext : public NonCopyable
	{
	public:
		FrameContext( VkRenderPass renderPass, const std::vector<VulkanImage> &images, glm::u32vec2 size );
		void Cleanup();
		VkFramebuffer GetFramebuffer() { return swapchainFramebuffer.framebuffer; }
		VkImage GetFramebufferImage() { return swapchainFramebuffer.swapchainImage; }
		VkCommandBuffer GetCommandBuffer() { return commandBuffer; }
	private:
		Framebuffer swapchainFramebuffer;
		VkCommandPool commandPool;
		VkCommandBuffer commandBuffer;
		Semaphores semaphores;
	};

    class RenderPass : public NonCopyable
    {
    public:
		void Create();
		void Cleanup();
		void CreateFrameContexts( const std::vector<VulkanImage> &images, VulkanImage &depthBuffer, glm::i32vec2 size );
		void RecordFrameDraw( /*Scene?*/ );
		FrameContext& GetFrameContext( uint32_t swapchainImageId );
		const VkRenderPass GetRenderPass() const { return renderPass; }

	private:
		VkRenderPass renderPass;
		std::vector<FrameContext> frameContexts;
		Pipeline pipeline;
    };
}