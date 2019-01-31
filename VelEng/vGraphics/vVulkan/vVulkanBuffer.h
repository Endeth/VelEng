#pragma once

#include "vVulkanCommon.h"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif

namespace Vel
{
	class VulkanBuffer
	{
	public:
		void CreateBuffer( VkBufferCreateFlags flags, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, const std::vector<uint32_t> &queueFamilyIndices );
		void AllocateMemory( uint32_t memoryTypeIndex );
		void CopyDataToBuffer( void* srcData, size_t size );
		void DestroyBuffer();

		VkBuffer _buffer = VK_NULL_HANDLE;
		VkDeviceMemory _bufferMemory = VK_NULL_HANDLE;
		VkMemoryRequirements _memoryRequirements;
	};
}