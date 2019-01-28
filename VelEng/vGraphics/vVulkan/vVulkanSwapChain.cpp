#include "vVulkanSwapChain.h"

namespace Vel
{
    void VulkanSwapchain::Init( GLFWwindow *window )
    {
		glfwGetWindowSize( window, &_imageSize.x, &_imageSize.y );
        CheckResult( glfwCreateWindowSurface( VulkanCommon::Instance, window, nullptr, &_surface ), "fail creating surface" ); //TODO get rid of glfw here
    }

    void VulkanSwapchain::Create( SwapchainSupportDetails swapchainSupport, uint32_t queueIndex )
    {
		for( auto &format : swapchainSupport.formats ) //TODO handle more/lack of specific
		{
			if( format.format == VK_FORMAT_B8G8R8A8_UNORM )
			{
				_format = format;
				break;
			}

		}

		for( auto &presentMode : swapchainSupport.presentModes ) //TODO handle more/lack of specific
		{
			if( presentMode == VK_PRESENT_MODE_FIFO_KHR )
			{
				_presentMode = presentMode;
				break;
			}

		}

		VkSwapchainCreateInfoKHR swapchainCreateInfo;
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.flags = 0;
		swapchainCreateInfo.pNext = nullptr;
		swapchainCreateInfo.surface = _surface;
		swapchainCreateInfo.minImageCount = 2; //TODO use physicaldevicesurfacecapabilities
		swapchainCreateInfo.imageFormat = _format.format;
		swapchainCreateInfo.imageColorSpace = _format.colorSpace;
		swapchainCreateInfo.imageExtent.width = _imageSize.x; //TODO use physicaldevicesurfacecapabilities
		swapchainCreateInfo.imageExtent.height = _imageSize.y; //TODO use physicaldevicesurfacecapabilities
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.preTransform = swapchainSupport.capabilities.currentTransform;
		swapchainCreateInfo.queueFamilyIndexCount = 1; //TODO maybe more queues
		swapchainCreateInfo.pQueueFamilyIndices = &queueIndex;
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.presentMode = _presentMode;
		swapchainCreateInfo.oldSwapchain = nullptr; 

		CheckResult( vkCreateSwapchainKHR( VulkanCommon::Device, &swapchainCreateInfo, nullptr, &_swapchain ), "fail to create swapchain" );

		vkGetSwapchainImagesKHR( VulkanCommon::Device, _swapchain, &_imageCount, nullptr );
		_images.resize( _imageCount );
		VkImage *swapchainImages = new VkImage[_imageCount];
		CheckResult( vkGetSwapchainImagesKHR( VulkanCommon::Device, _swapchain, &_imageCount, swapchainImages ), "failed to get swapchain images" );

		VkComponentMapping mapping;
		mapping.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		mapping.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		mapping.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		mapping.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		VkImageSubresourceRange subresourceRange;
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

		VkImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = _format.format;
		imageViewCreateInfo.components = mapping;
		imageViewCreateInfo.subresourceRange = subresourceRange;

		for( int i = 0; i < _imageCount; ++i )
		{
			_images[i].Image = swapchainImages[i];
			imageViewCreateInfo.image = _images[i].Image;

			CheckResult( vkCreateImageView( VulkanCommon::Device, &imageViewCreateInfo, nullptr, &_images[i].ImageView ), "failed to create image view" );
		}
		delete[] swapchainImages;
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
                //vkDestroyImageView( _deviceManager, _buffers[i].view, nullptr );
			vkDestroySwapchainKHR( VulkanCommon::Device, _swapchain, nullptr );
        }
        if ( _surface != VK_NULL_HANDLE )
        {
            vkDestroySurfaceKHR( VulkanCommon::Instance, _surface, nullptr );
        }
        _surface = VK_NULL_HANDLE;
        _swapchain = VK_NULL_HANDLE;
    }
}

