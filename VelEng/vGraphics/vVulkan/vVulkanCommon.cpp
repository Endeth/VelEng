#include "vVulkanCommon.h"

namespace Vel
{
	glm::i32vec2 VulkanOptions::WindowSize = { 0, 0 };
	VkInstance VulkanCommon::Instance = VK_NULL_HANDLE;
	VkDevice VulkanCommon::Device = VK_NULL_HANDLE;
	VkPhysicalDevice VulkanCommon::PhysicalDevice = VK_NULL_HANDLE;

	void Semaphores::Create() //TODO create proper semaphores
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo;
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;
		semaphoreCreateInfo.flags = 0;

		CheckResult( vkCreateSemaphore( VulkanCommon::Device, &semaphoreCreateInfo, nullptr, &renderComplete ), "failed to create render complete semaphore" );
		CheckResult( vkCreateSemaphore( VulkanCommon::Device, &semaphoreCreateInfo, nullptr, &acquireComplete ), "failed to create acquire complete semaphore" );
	}
	void Semaphores::Cleanup()
	{
		if( presentComplete != VK_NULL_HANDLE )
			vkDestroySemaphore( VulkanCommon::Device, presentComplete, nullptr );

		if( renderComplete != VK_NULL_HANDLE )
			vkDestroySemaphore( VulkanCommon::Device, renderComplete, nullptr );

		if( acquireComplete != VK_NULL_HANDLE )
			vkDestroySemaphore( VulkanCommon::Device, acquireComplete, nullptr );
	}
}