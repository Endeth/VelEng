#include "vVulkanDebug.h"

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


    VulkanDebug* VulkanDebug::_instance = nullptr;

    VulkanDebug::VulkanDebug( std::vector<const char*> &layers ) : _validationLayers( layers )
    {
        _cbCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        _cbCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        _cbCreateInfo.pfnCallback = Vel::VulkanDebug::DebugCallback;
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

    bool VulkanDebug::EnableValidationLayers( VkInstanceCreateInfo &createInfo )
    {
        std::vector<VkLayerProperties> availableLayers;
        VulkanQuery<VkLayerProperties>( vkEnumerateInstanceLayerProperties, availableLayers );

        std::cout << "Available Layers:" << std::endl;
        for ( auto &layer : availableLayers )
        {
            std::cout << "\t" << layer.layerName << std::endl;
        }
        std::cout << "Matching layers:" << std::endl;
        for ( const char *layerName : _validationLayers )
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
        createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
        createInfo.ppEnabledLayerNames = _validationLayers.data();

        return true;
    }

    bool VulkanDebug::EnableCallback( VkInstance &instance )
    {
        PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
        CreateDebugReportCallback = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr( instance, "vkCreateDebugReportCallbackEXT" ));
        if ( CreateDebugReportCallback != nullptr )
        {
            VkResult err = CreateDebugReportCallback( instance, &_cbCreateInfo, nullptr, &_dbCallback );
            assert( !err );
        }
        else
            return false;
        return true;
    }

    void VulkanDebug::DisableCallback( VkInstance & instance )
    {
        auto DestroyDebugReportCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr( instance, "vkDestroyDebugReportCallbackEXT" ));
        if ( DestroyDebugReportCallback != nullptr )
            DestroyDebugReportCallback( instance, _dbCallback, nullptr );
    }
}


