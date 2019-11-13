#pragma once

#include "vVulkanCommon.h"
#include "external/glfw/glfw3.h"
#include "external/glm/glm.hpp"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif


namespace Vel
{
    class Device
    {
    public:
        void Setup();
		void CreateDevice( bool useSwapChain = true, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT );
		void Destroy();
		void SetRequestedQueues( std::vector<VkDeviceQueueCreateInfo> &queueCreateInfos, VkQueueFlags queueType );

        //VkCommandPool CreateCommandPool( uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT );
		//void Create();

        class PhysicalDeviceProperties
        {
        public:
            void FindDevice();
            void QueryDevice();
			void QuerySwapchainSupport( VkSurfaceKHR surface );
			uint32_t FindMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties );

            VkBool32 GetSupportedDepthFormat( VkFormat *depthFormat );
            uint32_t GetQueueFamilyIndex( VkQueueFlagBits queueFlags );

            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceMemoryProperties memoryProperties;
            std::vector<VkQueueFamilyProperties> queueFamilyProperties;
            VkPhysicalDeviceFeatures features;

			SwapchainSupportDetails swapchainSupport;
		private:
			bool IsSuitable( VkPhysicalDevice device );

        } physicalDeviceProperties;

        QueueFamilyIndices queueFamilyIndices;
        Semaphores semaphores;

        VkFormat depthFormat;
        VkQueue gQueue;
        VkQueue pQueue;
        VkQueue tQueue;

        const float defaultQueuePriority = 0.5f;    
    };
}