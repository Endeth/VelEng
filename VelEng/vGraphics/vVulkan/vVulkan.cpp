#include "vVulkan.h"

namespace Vel
{
    void Vulkan::Init( GLFWwindow *window )
    {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan";
        appInfo.applicationVersion = VK_MAKE_VERSION( 0, 1, 0 );
        appInfo.pEngineName = "VelEng";
        appInfo.engineVersion = VK_MAKE_VERSION( 0, 1, 0 );
        appInfo.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
		createInfo.flags = 0;
		createInfo.pNext == nullptr;

        uint32_t glfwExtCount = 0;
        const char **glfwExtensions; //TODO
        glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtCount ); //TODO get rid of glfw here

        std::vector<const char*> extensions;
        extensions.insert( extensions.begin(), glfwExtensions, glfwExtensions + glfwExtCount );

#ifdef _DEBUG
		createInfo.enabledLayerCount = 1; //TODO check this out, few lines down there is also an assignement
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
        CheckResult( vkCreateInstance( &createInfo, nullptr, &_instance ), "failed to create instance");

        _device.Setup( _instance );

#ifdef _DEBUG
        if ( !VulkanDebug::Instance()->EnableCallback( _instance ) )
            throw std::runtime_error( "failed to set up debug callback" );
#endif

		CreateSurface( window );
    }

    void Vulkan::Destroy()
    {
#ifdef _DEBUG
        VulkanDebug::Instance()->DisableCallback( _instance );
#endif
		vkDestroyCommandPool( _device._logDevice, _commandPool, nullptr );
		_swapchain.Cleanup();
		_device.Destroy();
        vkDestroyInstance( _instance, nullptr );
    }

	void Vulkan::CreateSurface( GLFWwindow *window)
	{
		_swapchain.Init( _instance, _device._logDevice, window );

		VkBool32 presentSupported = 0;
		vkGetPhysicalDeviceSurfaceSupportKHR( _device._physicalDevice._suitableDevice, _device._queueFamilyIndices.graphics, _swapchain._surface, &presentSupported );

		_device._physicalDevice.QuerySwapchainSupport( _swapchain._surface );
		_swapchain.Create( _device._physicalDevice._swapchainSupport, _device._queueFamilyIndices.graphics );

		uint32_t swapchainImageCount = sizeof( _swapchainImages ) / sizeof( _swapchainImages[0] );
		CheckResult( vkGetSwapchainImagesKHR( _device._logDevice, _swapchain._swapchain, &swapchainImageCount, _swapchainImages ), "failed to get swapchain images" );

		VkCommandPoolCreateInfo cmdPoolCreateInfo;
		cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCreateInfo.pNext = nullptr;
		cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		cmdPoolCreateInfo.queueFamilyIndex = _device._queueFamilyIndices.graphics;

		CheckResult( vkCreateCommandPool( _device._logDevice, &cmdPoolCreateInfo, nullptr, &_commandPool ), "failed to crete command pool" ); //CREATION IN FUNC

		VkCommandBufferAllocateInfo commandBufferAllocateInfo;
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.commandPool = _commandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = 1;

		_commandBuffer = 0;
		vkAllocateCommandBuffers( _device._logDevice, &commandBufferAllocateInfo, &_commandBuffer );
	}
	void Vulkan::PresentImage()
	{
		uint32_t imageIndex = 0;
		CheckResult( vkAcquireNextImageKHR( _device._logDevice, _swapchain._swapchain, 0ull, _device._semaphores[0].acquireComplete, VK_NULL_HANDLE, &imageIndex ), "failed to acquire next image" );

		CheckResult( vkResetCommandPool( _device._logDevice, _commandPool, 0 ), "failed to reset cmd pool" );

		VkCommandBufferBeginInfo beginInfo;
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		CheckResult( vkBeginCommandBuffer( _commandBuffer, &beginInfo ), "failed to begin command buffer" );

		VkClearColorValue color = { 1, 0, 1, 1 };
		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		vkCmdClearColorImage( _commandBuffer, _swapchainImages[imageIndex], VK_IMAGE_LAYOUT_GENERAL, &color, 1, &range );

		CheckResult( vkEndCommandBuffer( _commandBuffer ), "failed to end command buffer");

		VkPipelineStageFlags submitStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &_device._semaphores[0].acquireComplete;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = &submitStageFlags;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &_commandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &_device._semaphores[0].renderComplete;

		vkQueueSubmit( _device._gQueue, 1, &submitInfo, VK_NULL_HANDLE );

		VkPresentInfoKHR presentInfo;
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &_device._semaphores[0].renderComplete;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &_swapchain._swapchain;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		CheckResult( vkQueuePresentKHR( _device._gQueue, &presentInfo ), "failed to queue present" );
		CheckResult( vkDeviceWaitIdle( _device._logDevice ), "failed to device wait idle" );
	}
}

