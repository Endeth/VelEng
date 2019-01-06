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
    class VulkanSwapChain
    {
    public:
        void InitSurface( GLFWwindow *platformHandle );
        void Connect( VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device );
        void Create( uint32_t *width, uint32_t *height, bool vsync = false );
        VkResult AcquireNextImage( VkSemaphore presentCompleteSemaphore, uint32_t *imageIndex );
        VkResult QueuePresent( VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE );
        void Cleanup();
    private:
        struct SwapChainBuffer
        {
            VkImage image;
            VkImageView view;
        };

        VkInstance _instance;
        VkPhysicalDevice _physicalDevice;
        VkDevice _device;

        VkSurfaceKHR _surface;
        VkFormat _colorFormat;
        VkColorSpaceKHR _colorSpace;
        
        VkSwapchainKHR _swapChain = VK_NULL_HANDLE;

        uint32_t _imageCount;
        std::vector<VkImage> _images;
        std::vector<SwapChainBuffer> _buffers;

        uint32_t _queueNodeIndex = UINT32_MAX;

        PFN_vkGetPhysicalDeviceSurfaceSupportKHR _getPhysicalDeviceSurfaceSupportKHR; //TODO physical device?
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