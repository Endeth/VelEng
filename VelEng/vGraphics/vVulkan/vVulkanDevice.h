#pragma once

#include "external/vulkan/vulkan.h"
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
        void SetupDevice( VkInstance &instance );
    private:
        class PhysicalDevice;
        class LogicalDevice;

        class PhysicalDevice 
        {
            friend class LogicalDevice;
        public:
            void FindDevice( VkInstance & instance );
            void QueryDevice( VkInstance & instance );

            VkBool32 GetSupportedDepthFormat( VkFormat *depthFormat );
        private:
            bool IsSuitable( VkPhysicalDevice device );
            uint32_t GetQueueFamilyIndex( VkQueueFlagBits queueFlags );

            VkPhysicalDeviceProperties _properties;
            VkPhysicalDeviceMemoryProperties _memoryProperties;
            std::vector<VkQueueFamilyProperties> _queueFamilyProperties;
            VkPhysicalDeviceFeatures _features;

            VkPhysicalDevice _suitableDevice = VK_NULL_HANDLE;
            VkCommandPool _commandPool = VK_NULL_HANDLE;
        } _GPU;

        class LogicalDevice
        {
            friend class PhysicalDevice;
        public:
            void CreateDevice( PhysicalDevice *pD, bool useSwapChain = true, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT );
            VkCommandPool CreateCommandPool( uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT );
            void CreateSemaphores();
        private:
            struct QueueFamilyIndices
            {
                uint32_t graphics;
                uint32_t present;
                uint32_t compute;
                uint32_t transfer;
            } _queueFamilyIndices;
            void RequestQueue( std::vector<VkDeviceQueueCreateInfo> &queueCreateInfos, VkQueueFlags queueType );

            struct Semaphores
            {
                VkSemaphore presentComplete;
                VkSemaphore renderComplete;
                VkSemaphore overlayComplete;
            } _semaphores[2];

            VkDevice _logDevice;
            VkFormat _depthFormat;
            VkCommandPool _gCommandPool;
            VkQueue _gQueue;
            VkQueue _pQueue;
            VkQueue _tQueue;
            VkQueue _cQueue;
            PhysicalDevice *_physicalDevice;

            const float _defaultQueuePriority = 0.5f;
        } _logical;
    };
}