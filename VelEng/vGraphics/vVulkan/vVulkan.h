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
        void Init();
        void Destroy();

    private:
        VulkanDevice _device;
        VulkanCommands _commands;
        VulkanSwapChain _swapChain;

        bool _enableValidationLayers = false;
        VkInstance _instance;

    };
}