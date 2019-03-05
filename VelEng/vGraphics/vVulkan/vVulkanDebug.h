#pragma once

#include <sstream>
#include <cassert>
#include <iostream>

#include "vVulkanCommon.h"


namespace Vel
{
    class VulkanDebug
    {
    public:
        static VulkanDebug* Instance()
        {
            if ( !_instance )
                _instance = new VulkanDebug();
            return _instance;
        }
        static void Destroy()
        {
            delete _instance;
        }

        VulkanDebug( const VulkanDebug& ) = delete;
        ~VulkanDebug();

        void PrintExtensions();
        bool EnableValidationLayers( VkInstanceCreateInfo &createInfo );

        bool EnableCallback();
        void DisableCallback();

        void SetLayersExtensions( std::vector<VkExtensionProperties>&& extensions ) { _layersExtensions = extensions; }
		void SetValidationLayers( std::vector<const char*> &validationLayers = std::vector<const char*>( { "VK_LAYER_LUNARG_standard_validation", "VK_LAYER_LUNARG_monitor" } ) );

        const std::vector<const char*>& GetValidationLayers() { return _validationLayers; }
        const std::vector<VkExtensionProperties>& GetLayersExtensions() { return _layersExtensions; }
        VkDebugReportCallbackCreateInfoEXT& GetDebugCallbackInfo() { return _cbCreateInfo; }

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback( VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char *layerPrefix, const char *msg, void *userData );
    private:
        static VulkanDebug* _instance;
        VulkanDebug();

        std::vector<const char*> _validationLayers;
        std::vector<VkExtensionProperties> _layersExtensions;
        VkDebugReportCallbackEXT _dbCallback;
        VkDebugReportCallbackCreateInfoEXT _cbCreateInfo;

		bool _enableValidationLayers = true;
		bool _validationLayersSet = false;
    };
}