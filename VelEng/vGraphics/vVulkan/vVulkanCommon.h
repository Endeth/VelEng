#pragma once

#include <vulkan/vulkan.hpp>
#include "external/glm/common.hpp"
#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"

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

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct Semaphores
	{
		void Create();
		void Cleanup();

		VkSemaphore presentComplete = VK_NULL_HANDLE; //TODO not used as of now
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

	class NonCopyable //TODO move to some helpers file
	{
	public:
		NonCopyable() = default;
		~NonCopyable() = default;
		NonCopyable( NonCopyable && ) = default;
		NonCopyable &operator=( NonCopyable && ) noexcept = default;

	private:
		NonCopyable( const NonCopyable & ) = delete;
		NonCopyable &operator=( const NonCopyable & ) = delete;
	};
}