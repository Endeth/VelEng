#pragma once

#include "vVulkanCommon.h"
#include "external/glfw/glfw3.h"
#include "external/glm/glm.hpp"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif


namespace Vel
{
    class VulkanDeviceManager
    {
    public:
        void Setup();
		void CreateDevice( bool useSwapChain = true, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT );
		void Destroy();
		void SetRequestedQueues( std::vector<VkDeviceQueueCreateInfo> &queueCreateInfos, VkQueueFlags queueType );

        //VkCommandPool CreateCommandPool( uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT );
		//void CreateSemaphores();

        class PhysicalDeviceProperties
        {
        public:
            void FindDevice();
            void QueryDevice();
			void QuerySwapchainSupport( VkSurfaceKHR surface );

            VkBool32 GetSupportedDepthFormat( VkFormat *depthFormat );
            uint32_t GetQueueFamilyIndex( VkQueueFlagBits queueFlags );

            VkPhysicalDeviceProperties _properties;
            VkPhysicalDeviceMemoryProperties _memoryProperties;
            std::vector<VkQueueFamilyProperties> _queueFamilyProperties;
            VkPhysicalDeviceFeatures _features;

			SwapchainSupportDetails _swapchainSupport;
		private:
			bool IsSuitable( VkPhysicalDevice device );

        } _physicalDeviceProperties;

        QueueFamilyIndices _queueFamilyIndices;
        Semaphores _semaphores[2];

        VkFormat _depthFormat;
        VkQueue _gQueue;
        VkQueue _pQueue;
        //VkQueue _tQueue;
        //VkQueue _cQueue;

        const float _defaultQueuePriority = 0.5f;    
    };
}