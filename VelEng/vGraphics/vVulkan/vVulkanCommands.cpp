#include "vVulkanCommands.h"

namespace Vel
{
    void VulkanCommands::CreateCommandPool( VkDevice device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags )
    {
        VkCommandPoolCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = flags;
        createInfo.queueFamilyIndex = queueFamilyIndex;

        CheckResult( vkCreateCommandPool( device, &createInfo, nullptr, &_pool ), "creating command pool failed" );

		_buffers.resize( 3, VK_NULL_HANDLE );
    }

    void VulkanCommands::CreateCommandBuffer( const VkDevice *device, uint32_t cmdBuf, const VkCommandBufferAllocateInfo *cmdBufInfo )
    {
        VkResult res;
        if ( cmdBufInfo )
        {
            CheckResult( vkAllocateCommandBuffers( *device, cmdBufInfo, &_buffers[cmdBuf] ), "allocating cmd buffer failed" );
            return;
        }

        VkCommandBufferAllocateInfo cmdInfo;
        cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdInfo.pNext = nullptr;
        cmdInfo.commandPool = _pool;
        cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdInfo.commandBufferCount = 1;

		CheckResult( vkAllocateCommandBuffers( *device, &cmdInfo, &_buffers[cmdBuf] ), "allocating cmd buffer failed" );
    }

	void VulkanCommands::Cleanup()
	{
	}

    void VulkanCommands::BeginCommandBuffer( uint32_t cmdBuf, VkCommandBufferBeginInfo * cmdBeginInfo )
    {
        VkResult res;
        if ( cmdBeginInfo )
        {
			CheckResult( vkBeginCommandBuffer( _buffers[cmdBuf], cmdBeginInfo ), "begin command buffer failed" );
            return;
        }

        VkCommandBufferInheritanceInfo cmdBufInhInfo;
        cmdBufInhInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        cmdBufInhInfo.pNext = nullptr;
        cmdBufInhInfo.renderPass = VK_NULL_HANDLE;
        cmdBufInhInfo.subpass = 0;
        cmdBufInhInfo.framebuffer = 0;
        cmdBufInhInfo.occlusionQueryEnable = VK_FALSE;
        cmdBufInhInfo.queryFlags = 0;
        cmdBufInhInfo.pipelineStatistics = 0;

        VkCommandBufferBeginInfo cmdInfo;
        cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdInfo.pNext = nullptr;
        cmdInfo.flags = 0;
        cmdInfo.pInheritanceInfo = &cmdBufInhInfo;


		CheckResult( vkBeginCommandBuffer( _buffers[cmdBuf], &cmdInfo ), "begin command buffer failed" );

    }
    
    void VulkanCommands::EndCommandBuffer( uint32_t cmdBuf )
    {
		CheckResult( vkEndCommandBuffer( _buffers[cmdBuf] ), "end buffer failed" );
    }
    
    void VulkanCommands::SubmitCommandBuffer( const VkQueue & queue, const VkCommandBuffer * cmdBuffers, const VkSubmitInfo * submitInfo, const VkFence & fence )
    {
    }
}
