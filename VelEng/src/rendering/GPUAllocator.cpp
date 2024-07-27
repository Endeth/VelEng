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
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = size;

    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = memoryUsage;
    vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

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
	VkImageCreateInfo createdImgInfo = {};
	createdImgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createdImgInfo.pNext = nullptr;
	createdImgInfo.imageType = VK_IMAGE_TYPE_2D;
	createdImgInfo.mipLevels = 1;
	createdImgInfo.arrayLayers = 1;
	createdImgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createdImgInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	createdImgInfo.extent = extent;
	createdImgInfo.format = format;
	createdImgInfo.usage = usage;

	return createdImgInfo;
}

VkImageViewCreateInfo Vel::GPUAllocator::BuildImageViewCreateInfo(VkFormat format, VkImageAspectFlags usage, VkImage image)
{
	VkImageViewCreateInfo createdImgViewInfo = {};
	createdImgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createdImgViewInfo.pNext = nullptr;
	createdImgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createdImgViewInfo.subresourceRange.baseMipLevel = 0;
	createdImgViewInfo.subresourceRange.levelCount = 1;
	createdImgViewInfo.subresourceRange.baseArrayLayer = 0;
	createdImgViewInfo.subresourceRange.layerCount = 1;

	createdImgViewInfo.format = format;
	createdImgViewInfo.image = image;
	createdImgViewInfo.subresourceRange.aspectMask = usage;

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

	VmaAllocationCreateInfo allocinfo = {};
	allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CHECK(vmaCreateImage(vmaAllocator, &imageCreateInfo, &allocinfo, &newImage.image, &newImage.allocation, nullptr));

	VkImageAspectFlags aspectFlag = format == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageViewCreateInfo imageViewCreateInfo = BuildImageViewCreateInfo(format, aspectFlag, newImage.image);
	imageViewCreateInfo.subresourceRange.levelCount = imageCreateInfo.mipLevels;

	VK_CHECK(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &newImage.imageView));

	return newImage;
}

Vel::AllocatedImage Vel::GPUAllocator::CreateImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped)
{
	size_t data_size = size.depth * size.width * size.height * 4;
	AllocatedBuffer uploadbuffer = CreateBuffer(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	memcpy(uploadbuffer.info.pMappedData, data, data_size);

	AllocatedImage newImage = CreateImage(size, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, mipmapped);

	submitter.Submit([&](VkCommandBuffer cmd) {
		TransitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = size;

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

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = nullptr;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &immediateFence));

	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = graphicsQueueFamily;

	VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &immediateCommandPool));

	VkCommandBufferAllocateInfo cmdAllocateInfo = {};
	cmdAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdAllocateInfo.pNext = nullptr;
	cmdAllocateInfo.commandBufferCount = 1;
	cmdAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdAllocateInfo.commandPool = immediateCommandPool;

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

	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;
	cmdBeginInfo.pInheritanceInfo = nullptr;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(immediateCommandBuffer, &cmdBeginInfo));

	function(immediateCommandBuffer);

	VK_CHECK(vkEndCommandBuffer(immediateCommandBuffer));

	VkCommandBufferSubmitInfo cmdSubmitInfo = {};
	cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	cmdSubmitInfo.pNext = nullptr;
	cmdSubmitInfo.commandBuffer = immediateCommandBuffer;
	cmdSubmitInfo.deviceMask = 0;

	VkSubmitInfo2 submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submit.pNext = nullptr;

	submit.waitSemaphoreInfoCount = 0;
	submit.pWaitSemaphoreInfos = nullptr;
	submit.signalSemaphoreInfoCount = 0;
	submit.pSignalSemaphoreInfos = nullptr;

	submit.commandBufferInfoCount = 1;
	submit.pCommandBufferInfos = &cmdSubmitInfo;

	VK_CHECK(vkQueueSubmit2(queue, 1, &submit, immediateFence));

	VK_CHECK(vkWaitForFences(device, 1, &immediateFence, true, 9999999999));
}
