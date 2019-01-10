#pragma once

#include <vulkan/vulkan.hpp>
#include "external/glfw/glfw3.h"
#include "external/glm/glm.hpp"

#include "vVulkanUtil.h"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif


namespace Vel
{
    class VulkanDevice
    {
    public:
        void Setup( VkInstance &instance );
		void CreateDevice( bool useSwapChain = true, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT );
		void Destroy();
		void SetRequestedQueues( std::vector<VkDeviceQueueCreateInfo> &queueCreateInfos, VkQueueFlags queueType );

        //VkCommandPool CreateCommandPool( uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT );
		//void CreateSemaphores();

        class PhysicalDevice 
        {
        public:
            void FindDevice( VkInstance &instance );
            void QueryDevice( VkInstance &instance );
			void QuerySwapchainSupport( VkSurfaceKHR surface );

            VkBool32 GetSupportedDepthFormat( VkFormat *depthFormat );
            bool IsSuitable( VkPhysicalDevice device );
            uint32_t GetQueueFamilyIndex( VkQueueFlagBits queueFlags );

            VkPhysicalDeviceProperties _properties;
            VkPhysicalDeviceMemoryProperties _memoryProperties;
            std::vector<VkQueueFamilyProperties> _queueFamilyProperties;
            VkPhysicalDeviceFeatures _features;

            VkPhysicalDevice _suitableDevice = VK_NULL_HANDLE;
            VkCommandPool _commandPool = VK_NULL_HANDLE;

			SwapchainSupportDetails _swapchainSupport;

        } _physicalDevice;

        QueueFamilyIndices _queueFamilyIndices;
        Semaphores _semaphores[2];

        VkDevice _logDevice;
        VkFormat _depthFormat;
        VkCommandPool _gCommandPool;
        VkQueue _gQueue;
        VkQueue _pQueue;
        VkQueue _tQueue;
        VkQueue _cQueue;

        const float _defaultQueuePriority = 0.5f;
    };
}