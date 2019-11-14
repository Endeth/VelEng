#include "vVulkanDebug.h"
#include "vVulkanCommon.h"

namespace Vel
{

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebug::DebugCallback( VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char * layerPrefix, const char * msg, void * userData )
    {
        std::string prefix( "" );
        if ( flags & VK_DEBUG_REPORT_ERROR_BIT_EXT )
            prefix += "ERROR:";
        if ( flags & VK_DEBUG_REPORT_WARNING_BIT_EXT )
            prefix += "WARNING:";
        if ( flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT )
            prefix += "PERFORMANCE:";
        if ( flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT )
            prefix += "INFO:";
        if ( flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT )
            prefix += "DEBUG:";

        std::stringstream debugMessage;
        debugMessage << prefix << " [" << layerPrefix << "] Code " << code << "\nMessage" << msg << "\n";

        if ( flags & VK_DEBUG_REPORT_ERROR_BIT_EXT ) {
            std::cerr << debugMessage.str() << "\n";
            assert( false );
        }
        else {
            std::cout << debugMessage.str() << "\n";
        }

        fflush( stdout );

        return VK_FALSE;
    }


    VulkanDebug* VulkanDebug::instance = nullptr;

    VulkanDebug::VulkanDebug()
    {
        cbCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        cbCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        cbCreateInfo.pfnCallback = Vel::VulkanDebug::DebugCallback;
    }

    VulkanDebug::~VulkanDebug()
    {
    }

    void VulkanDebug::PrintExtensions()
    {
        std::vector<VkExtensionProperties> avExtensions;
        VulkanQuery<const char*, VkExtensionProperties>( nullptr, vkEnumerateInstanceExtensionProperties, avExtensions );

        std::cout << "Available instance extensions:" << std::endl;
        for ( const auto& extension : avExtensions )
        {
            std::cout << "\t" << extension.extensionName << std::endl;
        }
    }

	void VulkanDebug::SetValidationLayers( std::vector<const char*>& layers )
	{
		validationLayers = layers;
		validationLayersSet = true;
	}

    bool VulkanDebug::EnableValidationLayers( VkInstanceCreateInfo &createInfo )
    {
		if( !validationLayersSet )
		{
			SetValidationLayers();
		}

        std::vector<VkLayerProperties> availableLayers;
        VulkanQuery<VkLayerProperties>( vkEnumerateInstanceLayerProperties, availableLayers );

        std::cout << "Available Layers:" << std::endl;
        for ( auto &layer : availableLayers )
        {
            std::cout << "\t" << layer.layerName << std::endl;
        }
        std::cout << "Matching layers:" << std::endl;
        for ( const char *layerName : validationLayers )
        {
            bool layerFound = false;
            for ( const auto &layerProperties : availableLayers )
            {
                if ( strcmp( layerName, layerProperties.layerName ) == 0 )
                {
                    std::cout << "\t" << layerProperties.layerName << std::endl;
                    layerFound = true;
                    break;
                }
            }
            if ( !layerFound )
                return false;
        }
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        return true;
    }

    bool VulkanDebug::EnableCallback()
    {
        PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
        CreateDebugReportCallback = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr( VulkanCommon::Instance, "vkCreateDebugReportCallbackEXT" ));
        if ( CreateDebugReportCallback != nullptr )
        {
            VkResult err = CreateDebugReportCallback( VulkanCommon::Instance, &cbCreateInfo, nullptr, &dbCallback );
            assert( !err );
        }
        else
            return false;
        return true;
    }

    void VulkanDebug::DisableCallback()
    {
        auto DestroyDebugReportCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr( VulkanCommon::Instance, "vkDestroyDebugReportCallbackEXT" ));
        if ( DestroyDebugReportCallback != nullptr )
            DestroyDebugReportCallback( VulkanCommon::Instance, dbCallback, nullptr );
    }
}


