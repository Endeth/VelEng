#pragma once

#include <vulkan/vulkan.hpp>
#include "external/glfw/glfw3.h"
#include "external/glm/glm.hpp"

#include "vVulkanCommands.h"
#include "vVulkanDevice.h"
#include "vVulkanSwapChain.h"
#include "vVulkanUtil.h"

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
		void PresentImage();

        VulkanDevice _device;
        VulkanCommands _commands;
        VulkanSwapchain _swapchain;

		VkImage _swapchainImages[16]; //TODO remove HACK
		VkCommandPool _commandPool = 0; //TODO get rid of this test cmdpool
		VkCommandBuffer _commandBuffer = 0;

        bool _enableValidationLayers = false;
        VkInstance _instance;

    };
}