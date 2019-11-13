#pragma once

#include <sstream>
#include <cassert>

#include "vVulkanCommon.h"
#include "external/glfw/glfw3.h"
#include "external/glm/glm.hpp"

#include "vVulkanImage.h"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif


namespace Vel
{
    class Swapchain
    {
    public:
        void CreateSurface( GLFWwindow *window ); //TODO init and create?
        void CreateSwapchain( SwapchainSupportDetails swapchainSupport, uint32_t queueIndex ); //TODO proper queues
        VkResult AcquireNextImage( uint32_t *imageIndex, VkSemaphore imageAcquiredSemaphore, VkFence fence );
        void Cleanup();

        VkSurfaceKHR surface;
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;

		VkSurfaceFormatKHR format;
		VkPresentModeKHR presentMode;
        
		glm::i32vec2 imageSize;
        uint32_t imageCount;
        std::vector<VulkanImage> images;
	private:
		uint32_t GetImagesCount( VkSurfaceCapabilitiesKHR &capabilities );
		VkExtent2D GetAppropriateExtent( VkSurfaceCapabilitiesKHR &capabilities );
		VkSurfaceFormatKHR FindAppropriateFormat( std::vector<VkSurfaceFormatKHR> &formats );
		VkImageUsageFlags FindAppropriateUsageFlags( VkSurfaceCapabilitiesKHR &capabilities );
		VkPresentModeKHR FindAppropriatePresentMode( std::vector<VkPresentModeKHR> &presentModes );
    };
}