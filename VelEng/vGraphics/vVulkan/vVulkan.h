#pragma once

#include "vVulkanCommon.h"
#include "external/glfw/glfw3.h"
#include "external/glm/glm.hpp"

#include "vVulkanCommands.h"
#include "vVulkanDevice.h"
#include "vVulkanSwapChain.h"
#include "vVulkanRenderpass.h"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif

namespace Vel
{
    class Vulkan
    {
    public:
        void Init( GLFWwindow *window );
        void Destroy();
		void CreateSurface( GLFWwindow *window );
		void CreateCommandBuffers();
		void DestroyCommandBuffers();
		void RecordCommandBuffers();
		void CreateBuffer();
		void Draw();

        VulkanDeviceManager _deviceManager;
        VulkanCommands _commands;
        VulkanSwapchain _swapchain;
		VulkanRenderPass _renderPass;

		VkCommandPool _commandPool = VK_NULL_HANDLE; //TODO get rid of this test cmdpool
		std::vector<VkCommandBuffer> _commandBuffers;
		VkBuffer _vertexBuffer;
		VkDeviceMemory _vertexBufferMemory;

        bool _enableValidationLayers = false;
    };
}