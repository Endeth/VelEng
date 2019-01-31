#include "vVulkanBuffer.h"

namespace Vel
{
	void VulkanBuffer::CreateBuffer( VkBufferCreateFlags flags, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, const std::vector<uint32_t> &queueFamilyIndices )
	{
		VkBufferCreateInfo vertexBufferCreateInfo;
		vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferCreateInfo.pNext = nullptr;
		vertexBufferCreateInfo.flags = flags;
		vertexBufferCreateInfo.size = size;
		vertexBufferCreateInfo.usage = usage;
		vertexBufferCreateInfo.sharingMode = sharingMode;
		vertexBufferCreateInfo.queueFamilyIndexCount = queueFamilyIndices.size();
		vertexBufferCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();

		CheckResult( vkCreateBuffer( VulkanCommon::Device, &vertexBufferCreateInfo, nullptr, &_buffer ), "failed to create vertex buffer" );

		vkGetBufferMemoryRequirements( VulkanCommon::Device, _buffer, &_memoryRequirements );
	}
	void VulkanBuffer::AllocateMemory( uint32_t memoryTypeIndex )
	{
		VkMemoryAllocateInfo memoryAllocInfo;
		memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocInfo.pNext = nullptr;
		memoryAllocInfo.allocationSize = _memoryRequirements.size;
		memoryAllocInfo.memoryTypeIndex = memoryTypeIndex;

		CheckResult( vkAllocateMemory( VulkanCommon::Device, &memoryAllocInfo, nullptr, &_bufferMemory ), "failed to allocate memory" );
		CheckResult( vkBindBufferMemory( VulkanCommon::Device, _buffer, _bufferMemory, 0 ), "failed to bind memory" );
	}
	void VulkanBuffer::CopyDataToBuffer( void * srcData, size_t size )
	{
		void *data;
		vkMapMemory( VulkanCommon::Device, _bufferMemory, 0, size, 0, &data );
		memcpy( data, srcData, size );
		vkUnmapMemory( VulkanCommon::Device, _bufferMemory );
	}
	void VulkanBuffer::DestroyBuffer()
	{
		if( _bufferMemory != VK_NULL_HANDLE )
		{
			vkFreeMemory( VulkanCommon::Device, _bufferMemory, nullptr );
			_bufferMemory = VK_NULL_HANDLE;
		}
		if( _buffer != VK_NULL_HANDLE )
		{
			vkDestroyBuffer( VulkanCommon::Device, _buffer, nullptr );
			_buffer = VK_NULL_HANDLE;
		}
	}
}
