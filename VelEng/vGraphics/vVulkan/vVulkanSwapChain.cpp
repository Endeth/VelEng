#include "vVulkanSwapChain.h"

namespace Vel
{
    void VulkanSwapChain::InitSurface( GLFWwindow *window )
    {
        if ( glfwCreateWindowSurface( _instance, window, nullptr, &_surface ) != VK_SUCCESS )
            throw std::runtime_error( "fail creating surface" );
    }

    void VulkanSwapChain::Connect( VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device )
    {
    }

    void VulkanSwapChain::Create( uint32_t * width, uint32_t * height, bool vsync )
    {
    }

    VkResult VulkanSwapChain::AcquireNextImage( VkSemaphore presentCompleteSemaphore, uint32_t * imageIndex )
    {
        return VkResult();
    }

    VkResult VulkanSwapChain::QueuePresent( VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore )
    {
        return VkResult();
    }

    void VulkanSwapChain::Cleanup()
    {
        if ( _swapChain != VK_NULL_HANDLE )
        {
            for ( uint32_t i = 0; i < _imageCount; i++ )
                vkDestroyImageView( _device, _buffers[i].view, nullptr );
        }
        if ( _surface != VK_NULL_HANDLE )
        {
            _destroySwapchainKHR( _device, _swapChain, nullptr ); //TODO
            vkDestroySurfaceKHR( _instance, _surface, nullptr );
        }
        _surface = VK_NULL_HANDLE;
        _swapChain = VK_NULL_HANDLE;
    }
}

