#pragma once

#include "external/vulkan/vulkan.h"

#include "vVulkanUtil.h"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif

namespace Vel
{
    class VulkanCommands
    {
    public:
        void CreateCommandPool( VkDevice device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0 );
        void CreateCommandBuffer( const VkDevice *device, uint32_t cmdBuf, const VkCommandBufferAllocateInfo *cmdBufInfo );

        void BeginCommandBuffer( uint32_t cmdBuf, VkCommandBufferBeginInfo *cmdBeginInfo = nullptr );
        void EndCommandBuffer( uint32_t cmdBuf );

        void SubmitCommandBuffer( const VkQueue &queue, const VkCommandBuffer *cmdBuffers, const VkSubmitInfo *submitInfo = nullptr, const VkFence &fence = VK_NULL_HANDLE );

        void ResetPool();
    private:
        VkCommandPool _pool; //TODO more for threads
        std::vector<VkCommandBuffer> _buffers = std::vector<VkCommandBuffer>(3);
    };
}