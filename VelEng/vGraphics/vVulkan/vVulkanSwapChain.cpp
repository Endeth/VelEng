#include "vVulkanSwapChain.h"

namespace Vel
{
    void Swapchain::CreateSurface( GLFWwindow *window )
    {
		glfwGetWindowSize( window, &imageSize.x, &imageSize.y ); //TODO does this sets? queries? why options are set here?
		VulkanOptions::WindowSize.x = imageSize.x;
		VulkanOptions::WindowSize.y = imageSize.y;
        CheckResult( glfwCreateWindowSurface( VulkanCommon::Instance, window, nullptr, &surface ), "fail creating surface" ); //TODO get rid of glfw here
    }

    void Swapchain::CreateSwapchain( SwapchainSupportDetails swapchainSupport, uint32_t queueIndex )
    {
		format = FindAppropriateFormat( swapchainSupport.formats );
		presentMode = FindAppropriatePresentMode( swapchainSupport.presentModes );
		VkImageUsageFlags usageFlags = FindAppropriateUsageFlags( swapchainSupport.capabilities );

		VkSwapchainCreateInfoKHR swapchainCreateInfo;
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.flags = 0;
		swapchainCreateInfo.pNext = nullptr;
		swapchainCreateInfo.surface = surface;
		swapchainCreateInfo.minImageCount = GetImagesCount( swapchainSupport.capabilities );
		swapchainCreateInfo.imageFormat = format.format;
		swapchainCreateInfo.imageColorSpace = format.colorSpace;
		swapchainCreateInfo.imageExtent.width = imageSize.x; //TODO use physicaldevicesurfacecapabilities
		swapchainCreateInfo.imageExtent.height = imageSize.y; //TODO use physicaldevicesurfacecapabilities
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = usageFlags;
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.preTransform = swapchainSupport.capabilities.currentTransform;
		swapchainCreateInfo.queueFamilyIndexCount = 1; //TODO maybe more queues
		swapchainCreateInfo.pQueueFamilyIndices = &queueIndex;
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.presentMode = presentMode;
		swapchainCreateInfo.oldSwapchain = nullptr; 

		CheckResult( vkCreateSwapchainKHR( VulkanCommon::Device, &swapchainCreateInfo, nullptr, &swapchain ), "fail to create swapchain" );

		vkGetSwapchainImagesKHR( VulkanCommon::Device, swapchain, &imageCount, nullptr );
		images.resize( imageCount );
		VkImage *swapchainImages = new VkImage[imageCount];
		CheckResult( vkGetSwapchainImagesKHR( VulkanCommon::Device, swapchain, &imageCount, swapchainImages ), "failed to get swapchain images" );

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
		imageViewCreateInfo.format = format.format;
		imageViewCreateInfo.components = mapping;
		imageViewCreateInfo.subresourceRange = subresourceRange;

		for( int i = 0; i < imageCount; ++i )
		{
			images[i].image = swapchainImages[i]; //TODO constructor based on VkImage?
			imageViewCreateInfo.image = images[i].image;

			CheckResult( vkCreateImageView( VulkanCommon::Device, &imageViewCreateInfo, nullptr, &images[i].imageView ), "failed to create image view" );
		}
		delete[] swapchainImages;
    }

    VkResult Swapchain::AcquireNextImage( uint32_t *imageIndex, VkSemaphore imageAcquiredSemaphore, VkFence fence ) //TODO
    {
		return vkAcquireNextImageKHR( VulkanCommon::Device, swapchain, std::numeric_limits<uint64_t>::max(), imageAcquiredSemaphore, fence, imageIndex );
    }

    void Swapchain::Cleanup()
    {
        if ( swapchain != VK_NULL_HANDLE )
        {
            for ( uint32_t i = 0; i < imageCount; i++ )
                vkDestroyImageView( VulkanCommon::Device, images[i].imageView, nullptr );

			vkDestroySwapchainKHR( VulkanCommon::Device, swapchain, nullptr );
        }
        if ( surface != VK_NULL_HANDLE )
        {
            vkDestroySurfaceKHR( VulkanCommon::Instance, surface, nullptr );
        }
        surface = VK_NULL_HANDLE;
        swapchain = VK_NULL_HANDLE;
    }

	uint32_t Swapchain::GetImagesCount( VkSurfaceCapabilitiesKHR &capabilities )
	{
		return 2; //TODO maybe check, maybe not, if device can't handle surface won't be created
	}

	VkExtent2D Swapchain::GetAppropriateExtent( VkSurfaceCapabilitiesKHR &capabilities )
	{
		return VkExtent2D();
	}

	VkSurfaceFormatKHR Swapchain::FindAppropriateFormat( std::vector<VkSurfaceFormatKHR> &formats )
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

	VkImageUsageFlags Swapchain::FindAppropriateUsageFlags( VkSurfaceCapabilitiesKHR &capabilities )
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

	VkPresentModeKHR Swapchain::FindAppropriatePresentMode( std::vector<VkPresentModeKHR> &presentModes )
	{
		VkPresentModeKHR desirablePresentMode = VK_PRESENT_MODE_FIFO_KHR;
		VkPresentModeKHR fallbackPresentMode = static_cast<VkPresentModeKHR>( -1 );
		for( auto &presentMode : presentModes )
		{
			if( presentMode == desirablePresentMode )
				return desirablePresentMode;
			if( presentMode == VK_PRESENT_MODE_MAILBOX_KHR )
				fallbackPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		}

		return fallbackPresentMode;
	}
}

