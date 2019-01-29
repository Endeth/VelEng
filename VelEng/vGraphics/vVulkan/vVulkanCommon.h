#pragma once

#include <vulkan/vulkan.hpp>
#include "external/glm/common.hpp"

#include "vVulkanUtil.h"
#include "VelEngConfig.h"

namespace Vel
{
	struct VulkanCommon
	{
		static VkInstance Instance;
		static VkDevice Device;
		static VkPhysicalDevice PhysicalDevice;
	};

	struct VulkanOptions
	{
		static glm::i32vec2 WindowSize;
	};

	struct VulkanImage
	{
		VkImage Image;
		VkImageView ImageView;
		VkSampler Sampler;
		VkDeviceMemory Memory;

		VulkanImage() :
			Image( VK_NULL_HANDLE ),
			ImageView( VK_NULL_HANDLE ),
			Sampler( VK_NULL_HANDLE ),
			Memory( VK_NULL_HANDLE )
		{}

		void DestroyImageView();
	};

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct Semaphores
	{
		void CreateSemaphores();
		void Cleanup();

		VkSemaphore presentComplete = VK_NULL_HANDLE;
		VkSemaphore renderComplete = VK_NULL_HANDLE;
		VkSemaphore acquireComplete = VK_NULL_HANDLE;
	};

	struct QueueFamilyIndices
	{
		uint32_t graphics = 0; //TODO init to -1?
		uint32_t present = 0;
		uint32_t compute = 0;
		uint32_t transfer = 0;
	};
}