#include "Rendering/Buffers/GPUAllocator.h"

#include "Rendering/VulkanUtils.h"

void Vel::GPUAllocator::Init(VkDevice dev, VmaAllocatorCreateInfo allocatorCreateInfo, uint32_t graphicsQueueFamily, VkQueue targetQueue)
{
	constexpr size_t stagingBufferSize = 2048U * 2048U * 4U;
    device = dev;
	vmaCreateAllocator(&allocatorCreateInfo, &vmaAllocator);
	submitter.Init(dev, graphicsQueueFamily, targetQueue);

	//stagingBuffer = CreateBuffer(stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	stagingBuffer = CreateBuffer(stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
}

void Vel::GPUAllocator::Cleanup()
{
	DestroyBuffer(stagingBuffer);
	submitter.Cleanup();
	vmaDestroyAllocator(vmaAllocator);
}

Vel::AllocatableBuffer Vel::GPUAllocator::CreateBuffer(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    VkBufferCreateInfo bufferInfo {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.size = size,
		.usage = usage
	};

    VmaAllocationCreateInfo vmaAllocInfo {
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = memoryUsage
	};

	AllocatableBuffer newBuffer;
    VK_CHECK(vmaCreateBuffer(vmaAllocator, &bufferInfo, &vmaAllocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.info));

    return newBuffer;
}

void Vel::GPUAllocator::DestroyBuffer(const AllocatableBuffer& buffer)
{
	vmaDestroyBuffer(vmaAllocator, buffer.buffer, buffer.allocation);
}

Vel::AllocatableBuffer& Vel::GPUAllocator::GetStagingBuffer()
{
	return stagingBuffer;
}

VkImageCreateInfo Vel::GPUAllocator::Create2DImageCreateInfo(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage)
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

VkImageCreateInfo Vel::GPUAllocator::CreateCubeImageCreateInfo(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage)
{
	VkImageCreateInfo createdImgInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,

		.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = extent,
		.mipLevels = 1,
		.arrayLayers = 6,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usage
	};

	return createdImgInfo;
}

VkImageViewCreateInfo Vel::GPUAllocator::Create2DImageViewCreateInfo(VkFormat format, VkImageAspectFlags usage, VkImage image)
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

VkImageViewCreateInfo Vel::GPUAllocator::CreateCubeImageViewCreateInfo(VkFormat format, VkImage image)
{
	VkImageViewCreateInfo createdImgViewInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,

		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_CUBE,
		.format = format,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 6
		}
	};

	return createdImgViewInfo;
}

VkBufferImageCopy Vel::GPUAllocator::CreateBufferImageCopy(VkExtent3D extent, uint32_t layersCount)
{
	return VkBufferImageCopy {
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = layersCount
		},
		.imageExtent = extent
	};
}

void Vel::GPUAllocator::AllocateImage(AllocatableImage& image)
{
	VkImageCreateInfo imageCreateInfo = Create2DImageCreateInfo(image.extent, image.format, image.usageFlags);

	VmaAllocationCreateInfo allocinfo{
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
		.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	VK_CHECK(vmaCreateImage(vmaAllocator, &imageCreateInfo, &allocinfo, &image.image, &image.allocation, nullptr));

	VkImageAspectFlags aspectFlag = image.format == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	VkImageViewCreateInfo imageViewCreateInfo = Create2DImageViewCreateInfo(image.format, aspectFlag, image.image);

	VK_CHECK(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &image.imageView));
}

Vel::AllocatableImage Vel::GPUAllocator::CreateAllocatableImage(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage)
{
	AllocatableImage newImage;
	newImage.format = format;
	newImage.extent = extent;
	newImage.usageFlags = usage;

	AllocateImage(newImage);

	return newImage;
}

Vel::AllocatableImage Vel::GPUAllocator::CreateImageFromData(void* data, VkExtent3D extent, VkFormat format, VkImageUsageFlags usage)
{
	//TODO staging buffer
	size_t dataSize = extent.depth * extent.width * extent.height * 4U;
	AllocatableBuffer uploadBuffer = CreateBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	memcpy(uploadBuffer.info.pMappedData, data, dataSize);

	AllocatableImage newImage = CreateAllocatableImage(extent, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

	submitter.Submit([&](VkCommandBuffer cmd) {
		TransitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copyRegion = CreateBufferImageCopy(extent, 1);
		vkCmdCopyBufferToImage(cmd, uploadBuffer.buffer, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		TransitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	});

	DestroyBuffer(uploadBuffer);

	return newImage;
}

Vel::AllocatableImage Vel::GPUAllocator::CreateAllocatableCubeImage(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage)
{
	AllocatableImage newImage;
	newImage.format = format;
	newImage.extent = extent;
	newImage.usageFlags = usage;

	VkImageCreateInfo imageCreateInfo = CreateCubeImageCreateInfo(extent, format, usage);

	VmaAllocationCreateInfo allocinfo{
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
		.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	VK_CHECK(vmaCreateImage(vmaAllocator, &imageCreateInfo, &allocinfo, &newImage.image, &newImage.allocation, nullptr));

	VkImageViewCreateInfo imageViewCreateInfo = CreateCubeImageViewCreateInfo(format, newImage.image);
	VK_CHECK(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &newImage.imageView));

	return newImage;
}

Vel::AllocatableImage Vel::GPUAllocator::CreateCubeImageFromData(std::array<unsigned char*, cubeTextureLayers> data , VkExtent3D extent, VkFormat format, VkImageUsageFlags usage)
{
	size_t layerSize = extent.width * extent.height * extent.depth * 4U;
	size_t dataSize = layerSize * cubeTextureLayers;
	AllocatableBuffer uploadBuffer = CreateBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	for (int i = 0; i < cubeTextureLayers; ++i)
	{
		unsigned char* layerMemory = static_cast<unsigned char*>(uploadBuffer.info.pMappedData);
		memcpy(layerMemory + (layerSize * i), data[i], layerSize);
	}

	AllocatableImage newImage = CreateAllocatableCubeImage(extent, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

	submitter.Submit([&](VkCommandBuffer cmd) {
		TransitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copyRegion = CreateBufferImageCopy(extent, 6);

		vkCmdCopyBufferToImage(cmd, uploadBuffer.buffer, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
		 
		TransitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	});

	DestroyBuffer(uploadBuffer);

	return newImage;
}

Vel::GPUMeshBuffers Vel::GPUAllocator::UploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices)
{
	const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
	const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

	GPUMeshBuffers newSurface;

	newSurface.vertexBuffer = CreateBuffer(vertexBufferSize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY);

	newSurface.indexBuffer = CreateBuffer(indexBufferSize,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY);

	VkBufferDeviceAddressInfo deviceAdressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = newSurface.vertexBuffer.buffer
	};
	newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(device, &deviceAdressInfo);

	//TODO create a single staging, resize it first launch, keep max size in config?
	AllocatableBuffer staging = CreateBuffer(vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	void* data = staging.info.pMappedData;

	memcpy(data, vertices.data(), vertexBufferSize);
	memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

	ImmediateSubmit([&](VkCommandBuffer cmd) {
		VkBufferCopy vertexCopy{
			.srcOffset = 0,
			.dstOffset = 0,
			.size = vertexBufferSize
		};

		vkCmdCopyBuffer(cmd, staging.buffer, newSurface.vertexBuffer.buffer, 1, &vertexCopy);

		VkBufferCopy indexCopy{
			.srcOffset = vertexBufferSize,
			.dstOffset = 0,
			.size = indexBufferSize
		};

		vkCmdCopyBuffer(cmd, staging.buffer, newSurface.indexBuffer.buffer, 1, &indexCopy);
	});

	DestroyBuffer(staging);

	return newSurface;
}

void Vel::GPUAllocator::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	submitter.Submit(std::move(function));
}

void Vel::GPUAllocator::DestroyImage(const AllocatableImage& image)
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
