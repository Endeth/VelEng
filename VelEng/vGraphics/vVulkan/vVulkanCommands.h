#pragma once

#include "vVulkanCommon.h"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif

namespace Vel
{
	void CreateCommandPool( VkCommandPoolCreateFlags createFlags, uint32_t queueIndex, VkCommandPool *cmdPool );
	void AllocateCommandBuffers( uint32_t count, VkCommandBuffer *buffers, VkCommandPool cmdPool, VkCommandBufferLevel level );

	VkCommandBuffer BeginSingleTimeCommand( VkCommandPool cmdPool );

	void CopyBufferCommand();
	void CopyImageCommand();
}