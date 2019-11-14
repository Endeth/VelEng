#include "vVulkanDevice.h"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif

namespace Vel
{
    bool VulkanDeviceManager::PhysicalDeviceProperties::IsSuitable( VkPhysicalDevice device )
    {
        vkGetPhysicalDeviceProperties( device, &properties );
        return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    }

	/*TODO checking for
    -supported extensions
    -queues*/
    void VulkanDeviceManager::PhysicalDeviceProperties::FindDevice()
    {

        std::vector<VkPhysicalDevice> devices;
        VulkanQuery<VkInstance, VkPhysicalDevice, VkResult>( VulkanCommon::Instance, vkEnumeratePhysicalDevices, devices );

        for ( const auto &device : devices )
        {
            if ( IsSuitable( device ) )
            {
				VulkanCommon::PhysicalDevice = device;
                break;
            }
        }

        if ( VulkanCommon::PhysicalDevice == VK_NULL_HANDLE )
            throw std::runtime_error( "failed to find a suitable GPU" );
    }

    void VulkanDeviceManager::PhysicalDeviceProperties::QueryDevice()
    {
        vkGetPhysicalDeviceFeatures( VulkanCommon::PhysicalDevice, &features );
        vkGetPhysicalDeviceMemoryProperties( VulkanCommon::PhysicalDevice, &memoryProperties );
        VulkanQuery<VkPhysicalDevice, VkQueueFamilyProperties>( VulkanCommon::PhysicalDevice, vkGetPhysicalDeviceQueueFamilyProperties, queueFamilyProperties ); //TODO handle wrong result
    }

	void VulkanDeviceManager::PhysicalDeviceProperties::QuerySwapchainSupport( VkSurfaceKHR surface )
	{
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR( VulkanCommon::PhysicalDevice, surface, &swapchainSupport.capabilities );
		VulkanQuery<VkPhysicalDevice, VkSurfaceKHR, VkSurfaceFormatKHR, VkResult>( VulkanCommon::PhysicalDevice, surface, vkGetPhysicalDeviceSurfaceFormatsKHR, swapchainSupport.formats ); //TODO handle wrong result
		VulkanQuery<VkPhysicalDevice, VkSurfaceKHR, VkPresentModeKHR, VkResult>( VulkanCommon::PhysicalDevice, surface, vkGetPhysicalDeviceSurfacePresentModesKHR, swapchainSupport.presentModes ); //TODO handle wrong result
	}

	uint32_t VulkanDeviceManager::PhysicalDeviceProperties::FindMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties )
	{
		for( uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i )
		{
			if( typeFilter & ( 1 << i ) && ( memoryProperties.memoryTypes[i].propertyFlags & properties ) == properties )
			{
				return i;
			}
		}
		return -1;
	}

    uint32_t VulkanDeviceManager::PhysicalDeviceProperties::GetQueueFamilyIndex( VkQueueFlagBits queueFlags ) //TODO try to find queues that are not already found
    {
        if ( queueFlags & VK_QUEUE_COMPUTE_BIT )
        {
            for ( uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++ )
            {
                if ( (queueFamilyProperties[i].queueFlags & queueFlags) && (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 )
                {
                    return i;
                }
            }
        }

        if( queueFlags & VK_QUEUE_TRANSFER_BIT )
        {
			uint32_t index = 10000;
            for( uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++ )
            {
                if( (queueFamilyProperties[i].queueFlags & queueFlags) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0) )
                {
                    return i;
                }
				else if( queueFamilyProperties[i].queueFlags & queueFlags )
				{
					index = i;
				}
            }
			if( index < 10000 )
				return index;
        }

        for ( uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++ )
        {
            if ( queueFamilyProperties[i].queueFlags & queueFlags )
            {
                return i;
            }
        }

        throw std::runtime_error( "Could not find a matching queue family index" );
    }

    VkBool32 VulkanDeviceManager::PhysicalDeviceProperties::GetSupportedDepthFormat( VkFormat * depthFormat )
    {
        std::vector<VkFormat> depthFormats = {
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D16_UNORM
        };

        for ( auto &format : depthFormats )
        {
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties( VulkanCommon::PhysicalDevice, format, &formatProperties );

            if ( formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT )
            {
                *depthFormat = format;
                return true;
            }
        }
        return false;
    }


	void VulkanDeviceManager::Setup()
	{
		physicalDeviceProperties.FindDevice();
		physicalDeviceProperties.QueryDevice();
		CreateDevice();
		semaphores.Create();
	}

	void VulkanDeviceManager::CreateDevice( bool useSwapChain, VkQueueFlags requestedQueueTypes )
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		SetRequestedQueues( queueCreateInfos, VK_QUEUE_GRAPHICS_BIT ); //TODO in device create info this q info has 0 flag - is that ok?
		SetRequestedQueues( queueCreateInfos, VK_QUEUE_TRANSFER_BIT );

		std::vector<const char*> deviceExtensions;
		if( useSwapChain )
			deviceExtensions.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );

		VkDeviceCreateInfo deviceCreateInfo;
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = nullptr;
		deviceCreateInfo.flags = 0;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>( queueCreateInfos.size() );
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.enabledLayerCount = 0;
		deviceCreateInfo.ppEnabledLayerNames = nullptr;
		deviceCreateInfo.enabledExtensionCount = 0;
		deviceCreateInfo.ppEnabledExtensionNames = nullptr;
		deviceCreateInfo.pEnabledFeatures = &( physicalDeviceProperties.features );

		if( deviceExtensions.size() > 0 )
		{
			deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>( deviceExtensions.size() );
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}

		CheckResult( vkCreateDevice( VulkanCommon::PhysicalDevice, &deviceCreateInfo, nullptr, &VulkanCommon::Device ), "failed to create device" );

		vkGetDeviceQueue( VulkanCommon::Device, queueFamilyIndices.graphics, 0, &gQueue ); //TODO move this someplace right
		vkGetDeviceQueue( VulkanCommon::Device, queueFamilyIndices.transfer, 0, &tQueue );
		//TODO compute & transfer

		if( !physicalDeviceProperties.GetSupportedDepthFormat( &depthFormat ) )
			throw std::runtime_error( "no suitable depth format" );

	}

	void VulkanDeviceManager::Destroy()
	{
		semaphores.Cleanup();

		vkDestroyDevice( VulkanCommon::Device, nullptr );
		VulkanCommon::Device = VK_NULL_HANDLE;
	}

    void VulkanDeviceManager::SetRequestedQueues( std::vector<VkDeviceQueueCreateInfo> &queueCreateInfos, VkQueueFlags queueType )
    {
        if ( queueType & VK_QUEUE_GRAPHICS_BIT )
        {
            queueFamilyIndices.graphics = physicalDeviceProperties.GetQueueFamilyIndex( VK_QUEUE_GRAPHICS_BIT );
            VkDeviceQueueCreateInfo queueInfo;
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.pNext = nullptr;
            queueInfo.flags = 0;
            queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            queueCreateInfos.push_back( queueInfo );
        }
		else if( queueType & VK_QUEUE_TRANSFER_BIT )
		{
			queueFamilyIndices.transfer = physicalDeviceProperties.GetQueueFamilyIndex( VK_QUEUE_TRANSFER_BIT );
			VkDeviceQueueCreateInfo queueInfo;
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.pNext = nullptr;
			queueInfo.flags = 0;
			queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back( queueInfo );
		}
        else
            queueFamilyIndices.graphics = VK_NULL_HANDLE;
    }
}



