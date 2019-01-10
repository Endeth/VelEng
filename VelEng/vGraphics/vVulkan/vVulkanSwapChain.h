#pragma once

#include <sstream>
#include <cassert>

#include <vulkan/vulkan.hpp>
#include "external/glfw/glfw3.h"
#include "external/glm/glm.hpp"

#include "vVulkanUtil.h"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif


namespace Vel
{
    class VulkanSwapchain
    {
    public:
        void Init( VkInstance instance, VkDevice device, GLFWwindow *window );
        void Connect();
        void Create( SwapchainSupportDetails swapchainSupport, uint32_t queueIndex ); //TODO proper queues
        VkResult AcquireNextImage( VkSemaphore presentCompleteSemaphore, uint32_t *imageIndex );
        VkResult QueuePresent( VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE );
        void Cleanup();

        struct SwapChainBuffer
        {
            VkImage image;
            VkImageView view;
        };

		VkInstance _instance;
		VkDevice _device;
		VkPhysicalDevice _physicalDevice;
		GLFWwindow *_window;

        VkSurfaceKHR _surface;
        VkFormat _colorFormat;
        VkColorSpaceKHR _colorSpace;
        
        VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

        uint32_t _imageCount;
        std::vector<VkImage> _images;
        std::vector<SwapChainBuffer> _buffers;

        uint32_t _queueNodeIndex = UINT32_MAX;

        PFN_vkGetPhysicalDeviceSurfaceSupportKHR _getPhysicalDeviceSurfaceSupportKHR;
        PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR _getPhysicalDeviceSurfaceCapabilitiesKHR;
        PFN_vkGetPhysicalDeviceSurfaceFormatsKHR _getPhysicalDeviceSurfaceFormatsKHR;
        PFN_vkGetPhysicalDeviceSurfacePresentModesKHR _getPhysicalDeviceSurfacePresentModesKHR;
        PFN_vkCreateSwapchainKHR _createSwapchainKHR;
        PFN_vkDestroySwapchainKHR _destroySwapchainKHR;
        PFN_vkGetSwapchainImagesKHR _getSwapchainImagesKHR;
        PFN_vkAcquireNextImageKHR _acquireNextImageKHR;
        PFN_vkQueuePresentKHR _queuePresentKHR;
    };
}