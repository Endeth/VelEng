#pragma once

#include "vVulkanCommon.h"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif

namespace Vel
{
	class VulkanBuffer //TODO inheritance (not too fast though)? not every buffer can be copied from etc
	{
	public:
		void CreateBuffer( VkBufferCreateFlags flags, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, const std::vector<uint32_t> &queueFamilyIndices );
		void AllocateMemory( uint32_t memoryTypeIndex );
		void CopyDataToBuffer( void* srcData, size_t size );
		void Destroy();

		VkBuffer _buffer = VK_NULL_HANDLE;
		VkDeviceMemory _bufferMemory = VK_NULL_HANDLE;
		VkMemoryRequirements memoryRequirements;
	};

	void CopyBuffer( VulkanBuffer &srcBuffer, VulkanBuffer &dstBuffer, VkDeviceSize size, VkCommandPool cmdPool, VkQueue transferQueue );
}