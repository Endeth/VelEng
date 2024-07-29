#include "GPUAllocator.h"
#include "VulkanUtils.h"

void Vel::GPUAllocator::Init(VkDevice dev, VmaAllocatorCreateInfo allocatorCreateInfo, uint32_t graphicsQueueFamily, VkQueue targetQueue)
{
    device = dev;
	vmaCreateAllocator(&allocatorCreateInfo, &vmaAllocator);
	submitter.Init(dev, graphicsQueueFamily, targetQueue);
}

void Vel::GPUAllocator::Cleanup()
{
	submitter.Cleanup();
	vmaDestroyAllocator(vmaAllocator);
}

Vel::AllocatedBuffer Vel::GPUAllocator::CreateBuffer(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    VkBufferCreateInfo bufferInfo {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.size = size,

		.usage = usage
	};

    VmaAllocationCreateInfo vmaallocInfo {
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = memoryUsage
	};

    AllocatedBuffer newBuffer;
    // allocate the buffer
    VK_CHECK(vmaCreateBuffer(vmaAllocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.info));

    return newBuffer;
}

void Vel::GPUAllocator::DestroyBuffer(const AllocatedBuffer& buffer)
{
	vmaDestroyBuffer(vmaAllocator, buffer.buffer, buffer.allocation);
}

VkImageCreateInfo Vel::GPUAllocator::BuildImageCreateInfo(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage)
{
	VkImageCreateInfo createdImgInfo {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,

		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = extent,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usage
	};

	return createdImgInfo;
}

VkImageViewCreateInfo Vel::GPUAllocator::BuildImageViewCreateInfo(VkFormat format, VkImageAspectFlags usage, VkImage image)
{
	VkImageViewCreateInfo createdImgViewInfo {

		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = {
			.aspectMask = usage,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	return createdImgViewInfo;
}

Vel::AllocatedImage Vel::GPUAllocator::CreateImage(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage, bool mipmapped)
{
	AllocatedImage newImage;
	newImage.imageFormat = format;
	newImage.imageExtent = extent;

	VkImageCreateInfo imageCreateInfo = BuildImageCreateInfo(extent, format, usage);
	if (mipmapped)
	{
		imageCreateInfo.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;
	}

	VmaAllocationCreateInfo allocinfo {
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
		.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	VK_CHECK(vmaCreateImage(vmaAllocator, &imageCreateInfo, &allocinfo, &newImage.image, &newImage.allocation, nullptr));

	VkImageAspectFlags aspectFlag = format == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageViewCreateInfo imageViewCreateInfo = BuildImageViewCreateInfo(format, aspectFlag, newImage.image);
	imageViewCreateInfo.subresourceRange.levelCount = imageCreateInfo.mipLevels;

	VK_CHECK(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &newImage.imageView));

	return newImage;
}

Vel::AllocatedImage Vel::GPUAllocator::CreateImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped)
{
	size_t data_size = size.depth * size.width * size.height * 4U;
	AllocatedBuffer uploadbuffer = CreateBuffer(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	memcpy(uploadbuffer.info.pMappedData, data, data_size);

	AllocatedImage newImage = CreateImage(size, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, mipmapped);

	submitter.Submit([&](VkCommandBuffer cmd) {
		TransitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copyRegion {
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.imageExtent = size
		};

		vkCmdCopyBufferToImage(cmd, uploadbuffer.buffer, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		TransitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	});

	DestroyBuffer(uploadbuffer);

	return newImage;
}

void Vel::GPUAllocator::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	submitter.Submit(std::move(function));
}

void Vel::GPUAllocator::DestroyImage(const AllocatedImage& image)
{
	vkDestroyImageView(device, image.imageView, nullptr);
	vmaDestroyImage(vmaAllocator, image.image, image.allocation);
}

void Vel::ImmediateSubmitter::Init(VkDevice dev, uint32_t graphicsQueueFamily, VkQueue targetQueue)
{
	device = dev;
	queue = targetQueue;

	VkFenceCreateInfo fenceInfo {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};


	VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &immediateFence));

	VkCommandPoolCreateInfo commandPoolInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = graphicsQueueFamily
	};

	VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &immediateCommandPool));

	VkCommandBufferAllocateInfo cmdAllocateInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = immediateCommandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocateInfo, &immediateCommandBuffer));
}

void Vel::ImmediateSubmitter::Cleanup()
{
	vkDestroyCommandPool(device, immediateCommandPool, nullptr);
	vkDestroyFence(device, immediateFence, nullptr);
}

void Vel::ImmediateSubmitter::Submit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	VK_CHECK(vkResetFences(device, 1, &immediateFence));
	VK_CHECK(vkResetCommandBuffer(immediateCommandBuffer, 0));

	VkCommandBufferBeginInfo cmdBeginInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = nullptr
	};

	VK_CHECK(vkBeginCommandBuffer(immediateCommandBuffer, &cmdBeginInfo));

	function(immediateCommandBuffer);

	VK_CHECK(vkEndCommandBuffer(immediateCommandBuffer));

	VkCommandBufferSubmitInfo cmdSubmitInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.pNext = nullptr,
		.commandBuffer = immediateCommandBuffer,
		.deviceMask = 0
	};

	VkSubmitInfo2 submit {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.pNext = nullptr,
		.waitSemaphoreInfoCount = 0,
		.pWaitSemaphoreInfos = nullptr,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cmdSubmitInfo,
		.signalSemaphoreInfoCount = 0,
		.pSignalSemaphoreInfos = nullptr
	};

	VK_CHECK(vkQueueSubmit2(queue, 1, &submit, immediateFence));

	VK_CHECK(vkWaitForFences(device, 1, &immediateFence, true, 9999999999));
}
