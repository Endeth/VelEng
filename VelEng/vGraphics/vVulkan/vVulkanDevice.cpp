#include "vVulkanDevice.h"
#include "vVulkanUtil.h"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif

namespace Vel
{
    bool VulkanDevice::PhysicalDevice::IsSuitable( VkPhysicalDevice device )
    {
        vkGetPhysicalDeviceProperties( device, &_properties );
        return _properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    }

	/*TODO checking for
    -supported extensions
    -queues*/
    void VulkanDevice::PhysicalDevice::FindDevice( VkInstance & instance )
    {

        std::vector<VkPhysicalDevice> devices;
        VulkanQuery<VkInstance, VkPhysicalDevice, VkResult>( instance, vkEnumeratePhysicalDevices, devices );

        for ( const auto &device : devices )
        {
            if ( IsSuitable( device ) )
            {
                _suitableDevice = device;
                break;
            }
        }

        if ( _suitableDevice == VK_NULL_HANDLE )
            throw std::runtime_error( "failed to find a suitable GPU" );
    }

    void VulkanDevice::PhysicalDevice::QueryDevice( VkInstance & instance )
    {
        vkGetPhysicalDeviceFeatures( _suitableDevice, &_features );
        vkGetPhysicalDeviceMemoryProperties( _suitableDevice, &_memoryProperties );
        VulkanQuery<VkPhysicalDevice, VkQueueFamilyProperties>( _suitableDevice, vkGetPhysicalDeviceQueueFamilyProperties, _queueFamilyProperties ); //TODO handle wrong result
    }

	void VulkanDevice::PhysicalDevice::QuerySwapchainSupport( VkSurfaceKHR surface )
	{
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR( _suitableDevice, surface, &_swapchainSupport.capabilities );
		VulkanQuery<VkPhysicalDevice, VkSurfaceKHR, VkSurfaceFormatKHR, VkResult>( _suitableDevice, surface, vkGetPhysicalDeviceSurfaceFormatsKHR, _swapchainSupport.formats ); //TODO handle wrong result
		VulkanQuery<VkPhysicalDevice, VkSurfaceKHR, VkPresentModeKHR, VkResult>( _suitableDevice, surface, vkGetPhysicalDeviceSurfacePresentModesKHR, _swapchainSupport.presentModes ); //TODO handle wrong result
	}

    uint32_t VulkanDevice::PhysicalDevice::GetQueueFamilyIndex( VkQueueFlagBits queueFlags )
    {
        if ( queueFlags & VK_QUEUE_COMPUTE_BIT )
        {
            for ( uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++ ) //COMPUTE
            {
                if ( (_queueFamilyProperties[i].queueFlags & queueFlags) && (_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 )
                {
                    return i;
                }
            }
        }

        if ( queueFlags & VK_QUEUE_TRANSFER_BIT )
        {
            for ( uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++ ) //TRANSFER
            {
                if ( (_queueFamilyProperties[i].queueFlags & queueFlags) && ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0) )
                {
                    return i;
                }
            }
        }

        for ( uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++ ) //GRAPHICS
        {
            if ( _queueFamilyProperties[i].queueFlags & queueFlags )
            {
                return i;
            }
        }

        throw std::runtime_error( "Could not find a matching queue family index" );
    }

    VkBool32 VulkanDevice::PhysicalDevice::GetSupportedDepthFormat( VkFormat * depthFormat )
    {
        std::vector<VkFormat> depthFormats = {
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM
        };

        for ( auto &format : depthFormats )
        {
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties( _suitableDevice, format, &formatProperties );

            if ( formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT )
            {
                *depthFormat = format;
                return true;
            }
        }
        return false;
    }


	void VulkanDevice::Setup( VkInstance & instance )
	{
		_physicalDevice.FindDevice( instance );
		_physicalDevice.QueryDevice( instance );
		CreateDevice();
		_semaphores[0].CreateSemaphores( _logDevice );
	}

	void VulkanDevice::CreateDevice( bool useSwapChain, VkQueueFlags requestedQueueTypes )
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		SetRequestedQueues( queueCreateInfos, VK_QUEUE_GRAPHICS_BIT );
		//SetRequestedQueues( queueCreateInfos, VK_QUEUE_COMPUTE_BIT ); //TODO
		//SetRequestedQueues( queueCreateInfos, VK_QUEUE_TRANSFER_BIT );
		//SetRequestedQueues( queueCreateInfos, VK_QUEUE_SPARSE_BINDING_BIT );

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
		deviceCreateInfo.pEnabledFeatures = &( _physicalDevice._features );

		if( deviceExtensions.size() > 0 )
		{
			deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>( deviceExtensions.size() );
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}

		VkResult result = vkCreateDevice( _physicalDevice._suitableDevice, &deviceCreateInfo, nullptr, &_logDevice );

		if( result == VK_SUCCESS )
		{
			//_gCommandPool = CreateCommandPool( _queueFamilyIndices.graphics );
		}

		vkGetDeviceQueue( _logDevice, _queueFamilyIndices.graphics, 0, &_gQueue ); //TODO move this someplace right
		//TODO compute & transfer

		if( !_physicalDevice.GetSupportedDepthFormat( &_depthFormat ) )
			throw std::runtime_error( "no suitable depth format" );

	}

	void VulkanDevice::Destroy()
	{
		vkDestroyDevice( _logDevice, nullptr );
	}

    void VulkanDevice::SetRequestedQueues( std::vector<VkDeviceQueueCreateInfo> &queueCreateInfos, VkQueueFlags queueType )
    {
        if ( queueType & VK_QUEUE_GRAPHICS_BIT )
        {
            _queueFamilyIndices.graphics = _physicalDevice.GetQueueFamilyIndex( VK_QUEUE_GRAPHICS_BIT );
            VkDeviceQueueCreateInfo queueInfo;
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.pNext = nullptr;
            queueInfo.flags = 0;
            queueInfo.queueFamilyIndex = _queueFamilyIndices.graphics;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &_defaultQueuePriority;
            queueCreateInfos.push_back( queueInfo );
        }
        else
            _queueFamilyIndices.graphics = VK_NULL_HANDLE;
    }
}



