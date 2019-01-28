#include "vVulkanUtil.h"

namespace Vel
{
	void CheckResult( VkResult result, const char *failMsg )
	{
		if( result != VK_SUCCESS )
		{
			throw std::runtime_error( failMsg );
		}
	}
}