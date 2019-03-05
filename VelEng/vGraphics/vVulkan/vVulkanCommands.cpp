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

	VkCommandBuffer BeginSingleTimeCommand( VkCommandPool cmdPool )
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.pNext = nullptr;
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = cmdPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer singleTimeCommandBuffer;
		vkAllocateCommandBuffers( VulkanCommon::Device, &allocInfo, &singleTimeCommandBuffer );

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		vkBeginCommandBuffer( singleTimeCommandBuffer, &beginInfo );

		return singleTimeCommandBuffer;
	}
}
