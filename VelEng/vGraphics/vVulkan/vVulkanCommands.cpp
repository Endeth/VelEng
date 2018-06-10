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

        auto res = vkCreateCommandPool( device, &createInfo, nullptr, &_pool );
        if( !res )
            throw std::runtime_error( "creating command pool failed" );

    }

    void VulkanCommands::CreateCommandBuffer( const VkDevice *device, uint32_t cmdBuf, const VkCommandBufferAllocateInfo *cmdBufInfo )
    {
        VkResult res;
        if ( cmdBufInfo )
        {
            res = vkAllocateCommandBuffers( *device, cmdBufInfo, &_buffers[cmdBuf] );
            if( !res )
                throw std::runtime_error( "allocating cmd buffer failed" );
            return;
        }

        VkCommandBufferAllocateInfo cmdInfo;
        cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdInfo.pNext = nullptr;
        cmdInfo.commandPool = _pool;
        cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdInfo.commandBufferCount = 1;

        res = vkAllocateCommandBuffers( *device, &cmdInfo, &_buffers[cmdBuf] );
        if ( !res )
            throw std::runtime_error( "allocating cmd buffer failed" );
    }

    void VulkanCommands::BeginCommandBuffer( uint32_t cmdBuf, VkCommandBufferBeginInfo * cmdBeginInfo )
    {
        VkResult res;
        if ( cmdBeginInfo )
        {
            res = vkBeginCommandBuffer( _buffers[cmdBuf], cmdBeginInfo );
            if ( !res )
                throw std::runtime_error( "begin command buffer failed" );
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


        res = vkBeginCommandBuffer( _buffers[cmdBuf], &cmdInfo );
        if ( !res )
            throw std::runtime_error( "begin command buffer failed" );

    }
    
    void VulkanCommands::EndCommandBuffer( uint32_t cmdBuf )
    {
        auto res = vkEndCommandBuffer( _buffers[cmdBuf] );
        if ( !res )
            throw std::runtime_error( "end buffer failed" );
    }
    
    void VulkanCommands::SubmitCommandBuffer( const VkQueue & queue, const VkCommandBuffer * cmdBuffers, const VkSubmitInfo * submitInfo, const VkFence & fence )
    {
    }
}

