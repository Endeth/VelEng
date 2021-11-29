#include "vVulkanRenderPass.h"
#include "vVulkanCommands.h"
#include "vVulkanDevice.h"

namespace Vel
{
	Framebuffer::Framebuffer( VkRenderPass renderPass, const std::vector<VulkanImage> &images, glm::u32vec2 size )
	{
		for( auto &img : images )
		{
			attachements.push_back( img.imageView );
			swapchainImage = img.image;

		}
		VkFramebufferCreateInfo framebufferCreateInfo;
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext = nullptr;
		framebufferCreateInfo.flags = 0;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = attachements.size();
		framebufferCreateInfo.pAttachments = attachements.data();
		framebufferCreateInfo.width = size.x;
		framebufferCreateInfo.height = size.y;
		framebufferCreateInfo.layers = 1;

		CheckResult( vkCreateFramebuffer( VulkanCommon::Device, &framebufferCreateInfo, nullptr, &framebuffer ), "failed to create framebuffer" );
	}

	void Framebuffer::Cleanup()
	{
		if( framebuffer != VK_NULL_HANDLE )
			vkDestroyFramebuffer( VulkanCommon::Device, framebuffer, nullptr );
	}

	FrameContext::FrameContext( VkRenderPass renderPass, const std::vector<VulkanImage> &images, glm::u32vec2 size ) : swapchainFramebuffer( renderPass, images, size )
	{
		CreateCommandPool( 0, VulkanCommon::DeviceManager.queueFamilyIndices.graphics, &commandPool );
		AllocateCommandBuffers( 1, &commandBuffer, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY );
		semaphores.Create();
	}

	void FrameContext::Cleanup()
	{
		swapchainFramebuffer.Cleanup();
		vkDestroyCommandPool( VulkanCommon::Device, commandPool, nullptr );
		commandPool = VK_NULL_HANDLE;
	}

	void RenderPass::Create()
	{
		std::array< VkAttachmentDescription, 2 > attachmentsDescriptions;
		attachmentsDescriptions[0].flags = 0;
		attachmentsDescriptions[0].format = VK_FORMAT_B8G8R8A8_UNORM; //TODO use somekind of getter from swapchain
		attachmentsDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentsDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentsDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentsDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentsDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentsDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentsDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		attachmentsDescriptions[1].flags = 0;
		attachmentsDescriptions[1].format = VK_FORMAT_D32_SFLOAT_S8_UINT; //TODO remove hack
		attachmentsDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentsDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentsDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentsDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentsDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentsDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentsDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentReference;
		colorAttachmentReference.attachment = 0;
		colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentReference;
		depthAttachmentReference.attachment = 1;
		depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription;
		subpassDescription.flags = 0;
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorAttachmentReference;
		subpassDescription.pResolveAttachments = nullptr;
		subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;

		VkSubpassDependency dependency;
		dependency.dependencyFlags = 0;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassCreateInfo = {};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.pNext = nullptr;
		renderPassCreateInfo.flags = 0;
		renderPassCreateInfo.attachmentCount = attachmentsDescriptions.size();
		renderPassCreateInfo.pAttachments = attachmentsDescriptions.data();
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &dependency;

		CheckResult( vkCreateRenderPass( VulkanCommon::Device, &renderPassCreateInfo, nullptr, &renderPass ), "failed to create renderpass" );
	}

	void RenderPass::CreateFrameContexts( const std::vector<VulkanImage> &images, VulkanImage &depthBuffer, glm::i32vec2 size )
	{
		for( auto &image : images )
			frameContexts.push_back( FrameContext( renderPass, std::vector<VulkanImage>( { image, depthBuffer } ), size ) );
	}

	void RenderPass::RecordFrameDraw( /*Scene?*/ )
	{
		VkCommandBufferBeginInfo beginInfo;
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		std::array<VkClearValue, 2> clearValues;
		clearValues[0].color = { 48.f / 256.f, 10 / 256.f, 36 / 256.f, 1.f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkImageSubresourceRange range; //not used as of now
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		VkRect2D renderArea;
		renderArea.offset = { 0, 0 };
		renderArea.extent = { static_cast<uint32_t>( VulkanOptions::WindowSize.x ), static_cast<uint32_t>( VulkanOptions::WindowSize.y ) };

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.renderArea = renderArea;
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		VkImageSubresourceRange subresourceRange;
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

		VkImageMemoryBarrier barrierFromPresentToDraw;
		barrierFromPresentToDraw.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrierFromPresentToDraw.pNext = nullptr;
		barrierFromPresentToDraw.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrierFromPresentToDraw.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrierFromPresentToDraw.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrierFromPresentToDraw.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrierFromPresentToDraw.srcQueueFamilyIndex = 0;
		barrierFromPresentToDraw.dstQueueFamilyIndex = 0;
		barrierFromPresentToDraw.subresourceRange = subresourceRange;

		VkImageMemoryBarrier barrierFromDrawToPresent;
		barrierFromDrawToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrierFromDrawToPresent.pNext = nullptr;
		barrierFromDrawToPresent.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrierFromDrawToPresent.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrierFromDrawToPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrierFromDrawToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrierFromDrawToPresent.srcQueueFamilyIndex = 0;
		barrierFromDrawToPresent.dstQueueFamilyIndex = 0;
		barrierFromDrawToPresent.subresourceRange = subresourceRange;


		for( auto& frameContext : frameContexts )
		{
			VkCommandBuffer frameCmdBuffer = frameContext.GetCommandBuffer();
			renderPassBeginInfo.framebuffer = frameContext.GetFramebuffer();
			barrierFromPresentToDraw.image = frameContext.GetFramebufferImage();
			barrierFromDrawToPresent.image = frameContext.GetFramebufferImage();

			CheckResult( vkBeginCommandBuffer( frameCmdBuffer, &beginInfo ), "failed to begin command buffer" );

			vkCmdPipelineBarrier( frameCmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierFromPresentToDraw ); //TODO think about moving this to framebuffer
			vkCmdBeginRenderPass( frameCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

			pipeline.Bind( frameCmdBuffer );

			/*
			vkCmdBindDescriptorSets( frameCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr ); //TODO
			Model->RenderCmd( frameCmdBuffer )
			vkCmdBindVertexBuffers( frameCmdBuffer, 0, 1, vertexBuffers, offsets );
			vkCmdBindIndexBuffer( frameCmdBuffer, indexBuffer._buffer, 0, VK_INDEX_TYPE_UINT32 );
			vkCmdDrawIndexed( frameCmdBuffer, testingModel->_meshes[0]->GetIndicesCount(), 1, 0, 0, 0 );
			*/


			vkCmdEndRenderPass( frameCmdBuffer );
			vkCmdPipelineBarrier( frameCmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierFromDrawToPresent );

			CheckResult( vkEndCommandBuffer( frameCmdBuffer ), "failed to end command buffer" );
		}
	}

	FrameContext& RenderPass::GetFrameContext( uint32_t swapchainImageId )
	{
		return frameContexts[swapchainImageId];
	}

	void RenderPass::Cleanup()
	{
		for( auto &frameContext : frameContexts )
			frameContext.Cleanup();

		vkDestroyRenderPass( VulkanCommon::Device, renderPass, nullptr );
		renderPass = VK_NULL_HANDLE;
	}
}