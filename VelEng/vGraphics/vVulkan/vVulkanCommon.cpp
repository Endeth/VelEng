#include "vVulkanCommon.h"

namespace Vel
{
	VkInstance VulkanCommon::Instance = VK_NULL_HANDLE;
	VkDevice VulkanCommon::Device = VK_NULL_HANDLE;
	VkPhysicalDevice VulkanCommon::PhysicalDevice = VK_NULL_HANDLE;

	void Semaphores::CreateSemaphores() //TODO create proper semaphores
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo;
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;
		semaphoreCreateInfo.flags = 0;

		CheckResult( vkCreateSemaphore( VulkanCommon::Device, &semaphoreCreateInfo, nullptr, &renderComplete ), "failed to create render complete semaphore" );
		CheckResult( vkCreateSemaphore( VulkanCommon::Device, &semaphoreCreateInfo, nullptr, &acquireComplete ), "failed to create acquire complete semaphore" );
	}
}