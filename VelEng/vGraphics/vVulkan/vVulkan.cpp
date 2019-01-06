#include "vVulkan.h"

namespace Vel
{
    void Vulkan::Init()
    {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan";
        appInfo.applicationVersion = VK_MAKE_VERSION( 0, 1, 0 );
        appInfo.pEngineName = "VelEng";
        appInfo.engineVersion = VK_MAKE_VERSION( 0, 1, 0 );
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        uint32_t glfwExtCount = 0;
        const char **glfwExtensions; //TODO
        glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtCount );

        std::vector<const char*> extensions;
        extensions.insert( extensions.begin(), glfwExtensions, glfwExtensions + glfwExtCount );

#ifdef _DEBUG
		createInfo.enabledLayerCount = 1;
        extensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
        VulkanDebug::Instance()->PrintExtensions();
        std::cout << "Used extensions:" << std::endl;
        for ( auto &ext : extensions )
        {
            std::cout << "\t" << ext << std::endl;
        }
        createInfo.enabledLayerCount = VulkanDebug::Instance()->GetLayersExtensions().size();
        if ( !VulkanDebug::Instance()->EnableValidationLayers( createInfo ) )
            throw std::runtime_error( "validation layers requested, but not available" );
#else
		createInfo.enabledLayerCount = 0;
#endif

        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        auto res = vkCreateInstance( &createInfo, nullptr, &_instance );
        if ( res != VK_SUCCESS ) {
            throw std::runtime_error( "failed to create instance" );
        }


        _device.Setup( _instance );

#ifdef _DEBUG
        if ( !VulkanDebug::Instance()->EnableCallback( _instance ) )
            throw std::runtime_error( "failed to set up debug callback" );
#endif
    }

    void Vulkan::Destroy()
    {
#ifdef _DEBUG
        VulkanDebug::Instance()->DisableCallback( _instance );
#endif
		_device.Destroy();
        vkDestroyInstance( _instance, nullptr );
    }
}

