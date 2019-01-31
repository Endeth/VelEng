#include "vVulkanCommands.h"

namespace Vel
{
	void CreateCommandPool( VkCommandPoolCreateFlags createFlags, uint32_t queueIndex, VkCommandPool * cmdPool )
	{
		VkCommandPoolCreateInfo cmdPoolCreateInfo;
		cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCreateInfo.pNext = nullptr;
		cmdPoolCreateInfo.flags = createFlags;
		cmdPoolCreateInfo.queueFamilyIndex = queueIndex;

		CheckResult( vkCreateCommandPool( VulkanCommon::Device, &cmdPoolCreateInfo, nullptr, cmdPool ), "failed to create command pool" );
	}
	void AllocateCommandBuffers( uint32_t count, VkCommandBuffer *buffers, VkCommandPool cmdPool, VkCommandBufferLevel level )
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo;
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.commandPool = cmdPool;
		commandBufferAllocateInfo.level = level;
		commandBufferAllocateInfo.commandBufferCount = count;

		CheckResult( vkAllocateCommandBuffers( VulkanCommon::Device, &commandBufferAllocateInfo, buffers ), "failed to allocate command buffers" );
	}
}
