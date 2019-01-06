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
    -surface formats
    -present modes
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

    /*
    -- Query for features, memory properties and queue properties --
    */
    void VulkanDevice::PhysicalDevice::QueryDevice( VkInstance & instance )
    {
        vkGetPhysicalDeviceFeatures( _suitableDevice, &_features );
        vkGetPhysicalDeviceMemoryProperties( _suitableDevice, &_memoryProperties );
        VulkanQuery<VkPhysicalDevice, VkQueueFamilyProperties>( _suitableDevice, vkGetPhysicalDeviceQueueFamilyProperties, _queueFamilyProperties );

/* TODO ?
#ifdef _DEBUG
        auto &layers = VulkanDebug::Instance()->GetValidationLayers();
        uint32_t size; //TODO
        std::vector<VkExtensionProperties> extensions;

        for ( auto &layer : layers )
        {
            std::vector<VkExtensionProperties> layerExt;
            vkEnumerateDeviceExtensionProperties( _suitableDevice, layer, &size, nullptr );
            layerExt.resize( size );
            vkEnumerateDeviceExtensionProperties( _suitableDevice, layer, &size, layerExt.data() );
        }

        std::cout << "Available device extensions:" << std::endl;
        for ( const auto& extension : extensions )
        {
            std::cout << "\t" << extension.extensionName << std::endl;
        }

        VulkanDebug::Instance()->SetLayersExtensions( std::move( extensions ) );
#endif*/

        //TODO vkGetPhysicalDeviceSurfaceFormats
        //TODO vkGetPhysicalDeviceSurfacePresentModes
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
	}

	void VulkanDevice::Destroy()
	{
		vkDestroyDevice( _logDevice, nullptr );
	}

    void VulkanDevice::RequestQueue( std::vector<VkDeviceQueueCreateInfo> &queueCreateInfos, VkQueueFlags queueType )
    {
        if ( queueType & VK_QUEUE_GRAPHICS_BIT ) //TODO make this smaller
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

    void VulkanDevice::CreateDevice( bool useSwapChain, VkQueueFlags requestedQueueTypes )
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        RequestQueue( queueCreateInfos, VK_QUEUE_GRAPHICS_BIT );
        //RequestQueue( queueCreateInfos, VK_QUEUE_COMPUTE_BIT ); //TODO
        //RequestQueue( queueCreateInfos, VK_QUEUE_TRANSFER_BIT );
        //RequestQueue( queueCreateInfos, VK_QUEUE_SPARSE_BINDING_BIT );

        std::vector<const char*> deviceExtensions;
        if ( useSwapChain )
            deviceExtensions.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );

        VkDeviceCreateInfo deviceCreateInfo;
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext = nullptr;
        deviceCreateInfo.enabledLayerCount = 0;
        deviceCreateInfo.ppEnabledLayerNames = nullptr;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.flags = 0;
        deviceCreateInfo.pEnabledFeatures = &( _physicalDevice._features);

        if ( deviceExtensions.size() > 0 )
        {
            deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
            deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        }

        VkResult result = vkCreateDevice( _physicalDevice._suitableDevice, &deviceCreateInfo, nullptr, &_logDevice );

        if ( result == VK_SUCCESS )
        {
            //_gCommandPool = CreateCommandPool( _queueFamilyIndices.graphics );
        }

        vkGetDeviceQueue( _logDevice, _queueFamilyIndices.graphics, 0, &_gQueue );
        //TODO compute & transfer

        if ( !_physicalDevice.GetSupportedDepthFormat( &_depthFormat ) )
            throw std::runtime_error( "no suitable depth format" );

    }

    VkCommandPool VulkanDevice::CreateCommandPool( uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags )
    {
        VkCommandPoolCreateInfo cmdPoolInfo;
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
        cmdPoolInfo.flags = createFlags;
        VkCommandPool cmdPool;
        //VkResult result = vkCreateCommandPool( _logDevice, &cmdPoolInfo, nullptr, &cmdPool );
        //if ( result != VK_SUCCESS )
        //    throw std::runtime_error( "error creating command pool" );
        return cmdPool;
    }

    void VulkanDevice::CreateSemaphores() //TODO get this straight
    {
        /*VkSemaphoreCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for ( int i = 0; i < 2; i++ )
        {
            vkCreateSemaphore( _logDevice, &createInfo, nullptr, &_semaphores[i].renderComplete );
            vkCreateSemaphore( _logDevice, &createInfo, nullptr, &_semaphores[i].presentComplete );
        }*/
    }
}



