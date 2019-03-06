#include "vVulkanImage.h"
#include "vVulkanCommands.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"

namespace Vel
{
	//std::array<VulkanSampler, VulkanSampler::Type::TYPES> Samplers{};
	void VulkanSamplers::CreateSamplers()
	{
		_samplers.resize( Type::TYPES );

		VkSamplerCreateInfo samplerCreateInfo;
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.pNext = nullptr;
		samplerCreateInfo.flags = 0;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.anisotropyEnable = VK_FALSE;
		samplerCreateInfo.maxAnisotropy = 1;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.mipLodBias = 0.f;
		samplerCreateInfo.minLod = 0.f;
		samplerCreateInfo.maxLod = 0.f;

		CheckResult( vkCreateSampler( VulkanCommon::Device, &samplerCreateInfo, nullptr, &_samplers[Type::BasicSampler] ), "failed to create sampler" );
	}

	void VulkanSamplers::DestroySamplers()
	{
		for( auto &sampler : _samplers )
		{
			vkDestroySampler( VulkanCommon::Device, sampler, nullptr );
		}
	}

	VkSampler VulkanSamplers::GetSampler( Type samplerType )
	{
		return _samplers[samplerType];
	}

	TexelData::TexelData( const char * path )
	{
		_data = stbi_load( path, &_imageSize.x, &_imageSize.y, &_imageChannels, STBI_rgb_alpha );
	}

	TexelData::~TexelData()
	{
		stbi_image_free( _data );
	}

	size_t TexelData::GetSize()
	{
		return _imageSize.x *_imageSize.y *_imageChannels;
	}

	void VulkanImage::Create( glm::ivec2 size, VkDeviceSize deviceSize, VkFormat imageFormat, VkImageCreateFlags flags, VkImageUsageFlags usage, VkSharingMode sharingMode, const std::vector<uint32_t>& queueFamilyIndices )
	{
		_imageSize = size;
		_format = imageFormat;
		VkImageCreateInfo imageCreateInfo;
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.flags = flags;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = _format;
		imageCreateInfo.extent.width = static_cast<uint32_t>( _imageSize.x );
		imageCreateInfo.extent.height = static_cast<uint32_t>( _imageSize.y );
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = usage;
		imageCreateInfo.sharingMode = sharingMode;
		imageCreateInfo.queueFamilyIndexCount = queueFamilyIndices.size();
		imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		CheckResult( vkCreateImage( VulkanCommon::Device, &imageCreateInfo, nullptr, &_image ), "failed to create image" );

		vkGetImageMemoryRequirements( VulkanCommon::Device, _image, &_memoryRequirements );
	}

	void VulkanImage::AllocateMemory( uint32_t memoryTypeIndex )
	{
		VkMemoryAllocateInfo memoryAllocInfo;
		memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocInfo.pNext = nullptr;
		memoryAllocInfo.allocationSize = _memoryRequirements.size;
		memoryAllocInfo.memoryTypeIndex = memoryTypeIndex;

		CheckResult( vkAllocateMemory( VulkanCommon::Device, &memoryAllocInfo, nullptr, &_imageMemory ), "failed to allocate memory" );
		CheckResult( vkBindImageMemory( VulkanCommon::Device, _image, _imageMemory, 0 ), "failed to bind memory" );

		VkImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.image = _image;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = _format;
		imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

		if( _format == VK_FORMAT_D32_SFLOAT_S8_UINT ) //TODO remove hack
			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		else
			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		CheckResult( vkCreateImageView( VulkanCommon::Device, &imageViewCreateInfo, nullptr, &_imageView ), "failed to create image view" );
	}

	void VulkanImage::AdjustMemoryBarrier( VkImageLayout newLayout, VkCommandPool cmdPool, VkQueue queue )
	{
		auto cmdBuffer = BeginSingleTimeCommand( cmdPool );
		VkImageSubresourceRange subresourceRange;
		if( _format == VK_FORMAT_D32_SFLOAT_S8_UINT ) //TODO remove hack
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		else
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.layerCount = 1;
		subresourceRange.levelCount = 1;

		VkImageMemoryBarrier imageBarrier;
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier.pNext = nullptr;
		imageBarrier.srcAccessMask = _accessMask;
		imageBarrier.oldLayout = _layout;
		imageBarrier.newLayout = newLayout;
		imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.image = _image;
		imageBarrier.subresourceRange = subresourceRange;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;

		if( _layout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
		{
			imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if( _layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
		{
			imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if( _layout == VK_FORMAT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
		{
			imageBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
		{
			throw std::invalid_argument( "Unsupported layout transition" );
		}

		_accessMask = imageBarrier.dstAccessMask; //TODO don't like it before submitting
		_layout = newLayout;

		vkCmdPipelineBarrier( cmdBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier );

		vkEndCommandBuffer( cmdBuffer );

		VkSubmitInfo submitInfo = {}; //TODO move submit to command buffer class
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr; //TODO use semaphore

		vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE );
		vkQueueWaitIdle( queue ); //TODO use synchronization
		vkFreeCommandBuffers( VulkanCommon::Device, cmdPool, 1, &cmdBuffer );
	}

	void VulkanImage::Destroy()
	{
		if( _imageView != VK_NULL_HANDLE )
		{
			vkDestroyImageView( VulkanCommon::Device, _imageView, nullptr );
			_imageView = VK_NULL_HANDLE;
		}
		if( _image != VK_NULL_HANDLE )
		{
			vkDestroyImage( VulkanCommon::Device, _image, nullptr );
			_image = VK_NULL_HANDLE;
		}
		if( _imageMemory != VK_NULL_HANDLE )
		{
			vkFreeMemory( VulkanCommon::Device, _imageMemory, nullptr );
			_imageMemory = VK_NULL_HANDLE;
		}
	}

	void CopyImage( VulkanImage &srcImage, VulkanImage &dstImage, VkDeviceSize size, VkCommandPool cmdPool, VkQueue transferQueue )
	{
		auto cmdBuffer = BeginSingleTimeCommand( cmdPool );

		VkImageCopy imageCopy;
		imageCopy.srcSubresource;
		imageCopy.srcOffset;
		imageCopy.dstSubresource;
		imageCopy.dstOffset;
		imageCopy.extent;

		vkCmdCopyImage( cmdBuffer, srcImage._image, srcImage._layout, dstImage._image, dstImage._layout, 1, &imageCopy );
		vkEndCommandBuffer( cmdBuffer );

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr; //TODO use semaphore

		vkQueueSubmit( transferQueue, 1, &submitInfo, VK_NULL_HANDLE );
		vkQueueWaitIdle( transferQueue ); //TODO use synchronization
		vkFreeCommandBuffers( VulkanCommon::Device, cmdPool, 1, &cmdBuffer );
	}

	void CopyImage( VulkanBuffer &srcBuffer, VulkanImage &dstImage, VkDeviceSize size, VkCommandPool cmdPool, VkQueue transferQueue )
	{
		auto cmdBuffer = BeginSingleTimeCommand( cmdPool );

		VkImageSubresourceLayers subresource;
		subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource.mipLevel = 0;
		subresource.baseArrayLayer = 0;
		subresource.layerCount = 1;

		VkBufferImageCopy imageCopy;
		imageCopy.bufferOffset = 0;
		imageCopy.bufferRowLength = 0;
		imageCopy.bufferImageHeight = 0;
		imageCopy.imageSubresource = subresource;
		imageCopy.imageOffset = { 0, 0, 0, };
		imageCopy.imageExtent.depth = 1;
		imageCopy.imageExtent.width = dstImage._imageSize.x;
		imageCopy.imageExtent.height = dstImage._imageSize.y;
 
		vkCmdCopyBufferToImage( cmdBuffer, srcBuffer._buffer, dstImage._image, dstImage._layout, 1, &imageCopy );
		vkEndCommandBuffer( cmdBuffer );

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr; //TODO use semaphore

		vkQueueSubmit( transferQueue, 1, &submitInfo, VK_NULL_HANDLE );
		vkQueueWaitIdle( transferQueue ); //TODO use synchronization
		vkFreeCommandBuffers( VulkanCommon::Device, cmdPool, 1, &cmdBuffer );
	}
}
