#include "vVulkanSwapChain.h"

namespace Vel
{
    void VulkanSwapchain::Init( GLFWwindow *window )
    {
		glfwGetWindowSize( window, &_imageSize.x, &_imageSize.y );
		VulkanOptions::WindowSize.x = _imageSize.x;
		VulkanOptions::WindowSize.y = _imageSize.y;
        CheckResult( glfwCreateWindowSurface( VulkanCommon::Instance, window, nullptr, &_surface ), "fail creating surface" ); //TODO get rid of glfw here
    }

    void VulkanSwapchain::Create( SwapchainSupportDetails swapchainSupport, uint32_t queueIndex )
    {
		_format = FindAppropriateFormat( swapchainSupport.formats );
		_presentMode = FindAppropriatePresentMode( swapchainSupport.presentModes );
		VkImageUsageFlags usageFlags = FindAppropriateUsageFlags( swapchainSupport.capabilities );

		VkSwapchainCreateInfoKHR swapchainCreateInfo;
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.flags = 0;
		swapchainCreateInfo.pNext = nullptr;
		swapchainCreateInfo.surface = _surface;
		swapchainCreateInfo.minImageCount = GetImagesCount( swapchainSupport.capabilities ); //TODO tbh might only be useful when implementing triple buffering
		swapchainCreateInfo.imageFormat = _format.format;
		swapchainCreateInfo.imageColorSpace = _format.colorSpace;
		swapchainCreateInfo.imageExtent.width = _imageSize.x; //TODO use physicaldevicesurfacecapabilities
		swapchainCreateInfo.imageExtent.height = _imageSize.y; //TODO use physicaldevicesurfacecapabilities
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = usageFlags;
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
			_images[i]._image = swapchainImages[i]; //TODO constructor based on VkImage?
			imageViewCreateInfo.image = _images[i]._image;

			CheckResult( vkCreateImageView( VulkanCommon::Device, &imageViewCreateInfo, nullptr, &_images[i]._imageView ), "failed to create image view" );
		}
		delete[] swapchainImages;
    }

    VkResult VulkanSwapchain::AcquireNextImage( VkSemaphore presentCompleteSemaphore, uint32_t * imageIndex ) //TODO
    {
        return VkResult();
    }

    VkResult VulkanSwapchain::QueuePresent( VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore ) //TODO
    {
        return VkResult();
    }

    void VulkanSwapchain::Cleanup()
    {
        if ( _swapchain != VK_NULL_HANDLE )
        {
            for ( uint32_t i = 0; i < _imageCount; i++ )
                vkDestroyImageView( VulkanCommon::Device, _images[i]._imageView, nullptr );

			vkDestroySwapchainKHR( VulkanCommon::Device, _swapchain, nullptr );
        }
        if ( _surface != VK_NULL_HANDLE )
        {
            vkDestroySurfaceKHR( VulkanCommon::Instance, _surface, nullptr );
        }
        _surface = VK_NULL_HANDLE;
        _swapchain = VK_NULL_HANDLE;
    }
	uint32_t VulkanSwapchain::GetImagesCount( VkSurfaceCapabilitiesKHR &capabilities )
	{
		return 2; //TODO maybe check, maybe not, if device can't handle surface won't be created
	}
	VkExtent2D VulkanSwapchain::GetAppropriateExtent( VkSurfaceCapabilitiesKHR &capabilities )
	{
		return VkExtent2D();
	}
	VkSurfaceFormatKHR VulkanSwapchain::FindAppropriateFormat( std::vector<VkSurfaceFormatKHR> &formats )
	{
		VkFormat desirableFormat = VK_FORMAT_B8G8R8A8_UNORM;
		if( ( formats.size() == 1 ) && ( formats[0].format == VK_FORMAT_UNDEFINED ) )
			return { desirableFormat, VK_COLORSPACE_SRGB_NONLINEAR_KHR };

		for( auto &format : formats )
		{
			if( format.format == desirableFormat )
				return format;
		}

		return formats[0];
	}
	VkImageUsageFlags VulkanSwapchain::FindAppropriateUsageFlags( VkSurfaceCapabilitiesKHR &capabilities )
	{
		if( capabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT )
			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		std::cout << "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT image usage is not supported by the swapchain" << std::endl
			<< "supported usage flags:" << std::endl
			<< ( capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT ? "    VK_IMAGE_USAGE_TRANSFER_SRC\n" : "" )
			<< ( capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT ? "    VK_IMAGE_USAGE_TRANSFER_DST\n" : "" )
			<< ( capabilities.supportedUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT ? "    VK_IMAGE_USAGE_SAMPLED\n" : "" )
			<< ( capabilities.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT ? "    VK_IMAGE_USAGE_STORAGE\n" : "" )
			<< ( capabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ? "    VK_IMAGE_USAGE_COLOR_ATTACHMENT\n" : "" )
			<< ( capabilities.supportedUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ? "    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT\n" : "" )
			<< ( capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT ? "    VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT\n" : "" )
			<< ( capabilities.supportedUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT ? "    VK_IMAGE_USAGE_INPUT_ATTACHMENT" : "" )
			<< std::endl;

		return static_cast<VkImageUsageFlags>( -1 );
	}
	VkPresentModeKHR VulkanSwapchain::FindAppropriatePresentMode( std::vector<VkPresentModeKHR> &presentModes )
	{
		VkPresentModeKHR desirablePresentMode = VK_PRESENT_MODE_FIFO_KHR;
		for( auto &presentMode : presentModes )
		{
			if( presentMode == desirablePresentMode )
				return desirablePresentMode;
		}

		for( auto &presentMode : presentModes )
		{
			if( presentMode == VK_PRESENT_MODE_MAILBOX_KHR )
				return VK_PRESENT_MODE_MAILBOX_KHR;
		}

		return static_cast<VkPresentModeKHR>( -1 );
	}
}

