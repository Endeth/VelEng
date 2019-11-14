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
            if ( !instance )
                instance = new VulkanDebug();
            return instance;
        }
        static void Destroy()
        {
            delete instance;
        }

        VulkanDebug( const VulkanDebug& ) = delete;
        ~VulkanDebug();

        void PrintExtensions();
        bool EnableValidationLayers( VkInstanceCreateInfo &createInfo );

        bool EnableCallback();
        void DisableCallback();

        void SetLayersExtensions( std::vector<VkExtensionProperties>&& extensions ) { layersExtensions = extensions; }
		void SetValidationLayers( std::vector<const char*> &layers = std::vector<const char*>( { "VK_LAYER_LUNARG_standard_validation", "VK_LAYER_LUNARG_monitor" } ) );

        const std::vector<const char*>& GetValidationLayers() { return validationLayers; }
        const std::vector<VkExtensionProperties>& GetLayersExtensions() { return layersExtensions; }
        VkDebugReportCallbackCreateInfoEXT& GetDebugCallbackInfo() { return cbCreateInfo; }

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback( VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char *layerPrefix, const char *msg, void *userData );
    private:
        static VulkanDebug* instance;
        VulkanDebug();

        std::vector<const char*> validationLayers;
        std::vector<VkExtensionProperties> layersExtensions;
        VkDebugReportCallbackEXT dbCallback;
        VkDebugReportCallbackCreateInfoEXT cbCreateInfo;

		bool enableValidationLayers = true;
		bool validationLayersSet = false;
    };
}