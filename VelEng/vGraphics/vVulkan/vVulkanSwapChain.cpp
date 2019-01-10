#include "vVulkanSwapChain.h"

namespace Vel
{
    void VulkanSwapchain::Init( VkInstance instance, VkDevice device, GLFWwindow *window )
    {
		_instance = instance;
		_device = device;
		_window = window;

        CheckResult( glfwCreateWindowSurface( _instance, _window, nullptr, &_surface ), "fail creating surface" ); //TODO get rid of glfw here
    }

    void VulkanSwapchain::Connect()
    {
    }

    void VulkanSwapchain::Create( SwapchainSupportDetails swapchainSupport, uint32_t queueIndex )
    {
		int winWidth = 0;
		int winHeight = 0;
		glfwGetWindowSize( _window, &winWidth, &winHeight );

		swapchainSupport.capabilities;

		VkSwapchainCreateInfoKHR swapchainCreateInfo;
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.flags = 0;
		swapchainCreateInfo.pNext = nullptr;
		swapchainCreateInfo.surface = _surface;
		swapchainCreateInfo.minImageCount = 2; //TODO use physicaldevicesurfacecapabilities
		swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM; //TODO use physicaldevicesurfaceformats
		swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		swapchainCreateInfo.imageExtent.width = winWidth; //TODO use physicaldevicesurfacecapabilities
		swapchainCreateInfo.imageExtent.height = winHeight; //TODO use physicaldevicesurfacecapabilities
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.preTransform = swapchainSupport.capabilities.currentTransform;
		swapchainCreateInfo.queueFamilyIndexCount = 1; //TODO maybe more queues
		swapchainCreateInfo.pQueueFamilyIndices = &queueIndex;
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; //TODO use physicaldevicesurfacecapabilities
		swapchainCreateInfo.oldSwapchain = nullptr;

		vkCreateSwapchainKHR( _device, &swapchainCreateInfo, nullptr, &_swapchain );
    }

    VkResult VulkanSwapchain::AcquireNextImage( VkSemaphore presentCompleteSemaphore, uint32_t * imageIndex )
    {
        return VkResult();
    }

    VkResult VulkanSwapchain::QueuePresent( VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore )
    {
        return VkResult();
    }

    void VulkanSwapchain::Cleanup()
    {
        if ( _swapchain != VK_NULL_HANDLE )
        {
            //for ( uint32_t i = 0; i < _imageCount; i++ )
                //vkDestroyImageView( _device, _buffers[i].view, nullptr );
			vkDestroySwapchainKHR( _device, _swapchain, nullptr );
        }
        if ( _surface != VK_NULL_HANDLE )
        {
            vkDestroySurfaceKHR( _instance, _surface, nullptr );
        }
        _surface = VK_NULL_HANDLE;
        _swapchain = VK_NULL_HANDLE;
    }
}

