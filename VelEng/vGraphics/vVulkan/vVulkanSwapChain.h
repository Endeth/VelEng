#pragma once

#include <sstream>
#include <cassert>

#include "vVulkanCommon.h"
#include "external/glfw/glfw3.h"
#include "external/glm/glm.hpp"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif


namespace Vel
{
    class VulkanSwapchain
    {
    public:
        void Init( GLFWwindow *window ); //TODO init and create?
        void Create( SwapchainSupportDetails swapchainSupport, uint32_t queueIndex ); //TODO proper queues
        VkResult AcquireNextImage( VkSemaphore presentCompleteSemaphore, uint32_t *imageIndex );
        VkResult QueuePresent( VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE );
        void Cleanup();

		GLFWwindow *_window;

        VkSurfaceKHR _surface;
		VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

		VkSurfaceFormatKHR _format;
		VkPresentModeKHR _presentMode;
        
		glm::i32vec2 _imageSize;
        uint32_t _imageCount;
        std::vector<VulkanImage> _images;
    };
}