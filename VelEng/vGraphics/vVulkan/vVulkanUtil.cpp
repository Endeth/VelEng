#include "vVulkanUtil.h"

namespace Vel
{
	void Semaphores::CreateSemaphores( VkDevice device ) //TODO create proper semaphores
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo;
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;
		semaphoreCreateInfo.flags = 0;

		CheckResult( vkCreateSemaphore( device, &semaphoreCreateInfo, nullptr, &renderComplete ), "failed to create render complete semaphore" );
		CheckResult( vkCreateSemaphore( device, &semaphoreCreateInfo, nullptr, &acquireComplete ), "failed to create acquire complete semaphore" );
	}
	void CheckResult( VkResult result, const char *failMsg )
	{
		if( result != VK_SUCCESS )
		{
			throw std::runtime_error( failMsg );
		}
	}
}