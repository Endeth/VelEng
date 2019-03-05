#include "vVulkanBuffer.h"
#include "vVulkanCommands.h"

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

		VkMappedMemoryRange flushRange;
		flushRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		flushRange.pNext = nullptr;
		flushRange.memory = _bufferMemory;
		flushRange.offset = 0;
		flushRange.size = VK_WHOLE_SIZE; //TODO check nonCoherentAtomSize
		vkFlushMappedMemoryRanges( VulkanCommon::Device, 1, &flushRange );

		vkUnmapMemory( VulkanCommon::Device, _bufferMemory );
	}
	void VulkanBuffer::Destroy()
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
		auto cmdBuffer = BeginSingleTimeCommand( cmdPool );

		VkBufferCopy bufferCopy;
		bufferCopy.size = size;
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = 0;

		vkCmdCopyBuffer( cmdBuffer, srcBuffer._buffer, dstBuffer._buffer, 1, &bufferCopy );

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
