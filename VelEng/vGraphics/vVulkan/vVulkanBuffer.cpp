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
	void CopyBuffer( VulkanBuffer &srcBuffer, VulkanBuffer &dstBuffer, VkDeviceSize size, VkCommandPool cmdPool, VkQueue transferQueue )
	{
		VkCommandBufferAllocateInfo cmdBufferAllocateInfo;
		cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocateInfo.pNext = nullptr;
		cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocateInfo.commandBufferCount = 1;
		cmdBufferAllocateInfo.commandPool = cmdPool;

		VkCommandBuffer cmdBuffer;
		CheckResult( vkAllocateCommandBuffers( VulkanCommon::Device, &cmdBufferAllocateInfo, &cmdBuffer ), "failed to allocate command buffer" );

		VkCommandBufferBeginInfo beginInfo;
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		VkBufferCopy bufferCopy;
		bufferCopy.size = size;
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = 0;

		vkBeginCommandBuffer( cmdBuffer, &beginInfo );
		vkCmdCopyBuffer( cmdBuffer, srcBuffer._buffer, dstBuffer._buffer, 1, &bufferCopy );
		vkEndCommandBuffer( cmdBuffer );

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		vkQueueSubmit( transferQueue, 1, &submitInfo, VK_NULL_HANDLE );
		vkQueueWaitIdle( transferQueue );
		vkFreeCommandBuffers( VulkanCommon::Device, cmdPool, 1, &cmdBuffer );
	}
}
