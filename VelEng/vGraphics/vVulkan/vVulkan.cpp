#include "vVulkan.h"
#include "vVulkanCommon.h"

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
        CheckResult( vkCreateInstance( &createInfo, nullptr, &VulkanCommon::Instance ), "failed to create instance");

#ifdef _DEBUG
		if( !VulkanDebug::Instance()->EnableCallback() )
			throw std::runtime_error( "failed to set up debug callback" );
#endif

        _deviceManager.Setup();

		CreateSurface( window );
		CreateBuffer();
		CreateCommandBuffers();
		_renderPass.Create();
		_renderPass.CreatePipeline();
		_renderPass.CreateFramebuffers( _swapchain._images, _swapchain._imageSize );
		RecordCommandBuffers();
    }

    void Vulkan::Destroy()
    {
#ifdef _DEBUG
        VulkanDebug::Instance()->DisableCallback();
#endif
		vkDestroyBuffer( VulkanCommon::Device, _vertexBuffer, nullptr );
		vkFreeMemory( VulkanCommon::Device, _vertexBufferMemory, nullptr );
		vkDestroyCommandPool( VulkanCommon::Device, _commandPool, nullptr );
		_commandPool = VK_NULL_HANDLE;

		_swapchain.Cleanup();
		_renderPass.Cleanup();
		_deviceManager.Destroy();

        vkDestroyInstance( VulkanCommon::Instance, nullptr );
		VulkanCommon::Instance = VK_NULL_HANDLE;
    }

	void Vulkan::CreateSurface( GLFWwindow *window)
	{
		_swapchain.Init( window );

		VkBool32 presentSupported = 0; //Create Proper Present Queue
		vkGetPhysicalDeviceSurfaceSupportKHR( VulkanCommon::PhysicalDevice, _deviceManager._queueFamilyIndices.graphics, _swapchain._surface, &presentSupported );

		_deviceManager._physicalDeviceProperties.QuerySwapchainSupport( _swapchain._surface );
		_swapchain.Create( _deviceManager._physicalDeviceProperties._swapchainSupport, _deviceManager._queueFamilyIndices.graphics );
	}

	void Vulkan::CreateCommandBuffers()
	{
		VkCommandPoolCreateInfo cmdPoolCreateInfo;
		cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCreateInfo.pNext = nullptr;
		cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		cmdPoolCreateInfo.queueFamilyIndex = _deviceManager._queueFamilyIndices.graphics;

		CheckResult( vkCreateCommandPool( VulkanCommon::Device, &cmdPoolCreateInfo, nullptr, &_commandPool ), "failed to create command pool" );

		uint32_t imagesCount = _swapchain._images.size();

		_commandBuffers.resize( imagesCount, VK_NULL_HANDLE );

		VkCommandBufferAllocateInfo commandBufferAllocateInfo;
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.commandPool = _commandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = imagesCount;

		CheckResult( vkAllocateCommandBuffers( VulkanCommon::Device, &commandBufferAllocateInfo, _commandBuffers.data() ), "failed to allocate command buffers" );
	}

	void Vulkan::RecordCommandBuffers()
	{
		VkCommandBufferBeginInfo beginInfo;
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		VkClearValue color = { 48.f / 256.f, 10 / 256.f, 36 / 256.f, 1.f };
		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		VkRect2D renderArea;
		renderArea.offset = { 0, 0 };
		renderArea.extent = { 500, 500 };

		uint32_t imageCount = _swapchain._images.size();

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = _renderPass._renderPass;
		renderPassBeginInfo.renderArea = renderArea;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &color;

		VkBuffer vertexBuffers[] = { _vertexBuffer };
		VkDeviceSize offsets[] = { 0 };

		for( int i = 0; i < imageCount; ++i )
		{
			CheckResult( vkBeginCommandBuffer( _commandBuffers[i], &beginInfo ), "failed to begin command buffer" );

			renderPassBeginInfo.framebuffer = _renderPass._framebuffers[i]._framebuffer;
			vkCmdBeginRenderPass( _commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
			vkCmdBindPipeline( _commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _renderPass._graphicsPipeline );
			vkCmdBindVertexBuffers( _commandBuffers[i], 0, 1, vertexBuffers, offsets );
			vkCmdDraw( _commandBuffers[i], 3, 1, 0, 0 );
			vkCmdEndRenderPass( _commandBuffers[i] );

			//vkCmdClearColorImage( _commandBuffers[i], _swapchain._images[i].Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1, &range );

			CheckResult( vkEndCommandBuffer( _commandBuffers[i] ), "failed to end command buffer" );
		}
	}

	void Vulkan::CreateBuffer()
	{
		VertexColor bufferData[3] = {
			VertexColor( glm::vec3( -1.f, -1.f, 0.f), glm::vec4( 1.f, 0.f, 0.f, 1.f ) ),
			VertexColor( glm::vec3( 1.f, 1.f, 0.f ), glm::vec4( 0.f, 1.f, 0.f, 1.f ) ),
			VertexColor( glm::vec3( -1.f, 1.f, 0.f ), glm::vec4( 0.f, 0.f, 1.f, 1.f ) ),
		};

		VkBufferCreateInfo vertexBufferCreateInfo;
		vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferCreateInfo.pNext = nullptr;
		vertexBufferCreateInfo.flags = 0;
		vertexBufferCreateInfo.size = sizeof(VertexColor) * 3;
		vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		vertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vertexBufferCreateInfo.queueFamilyIndexCount = 0;
		vertexBufferCreateInfo.pQueueFamilyIndices = nullptr;

		CheckResult( vkCreateBuffer( VulkanCommon::Device, &vertexBufferCreateInfo, nullptr, &_vertexBuffer ), "failed to create vertex buffer" );

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements( VulkanCommon::Device, _vertexBuffer, &memoryRequirements );

		VkMemoryAllocateInfo memoryAllocInfo;
		memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocInfo.pNext = nullptr;
		memoryAllocInfo.allocationSize = memoryRequirements.size;
		memoryAllocInfo.memoryTypeIndex = _deviceManager._physicalDeviceProperties.FindMemoryType( memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

		CheckResult( vkAllocateMemory( VulkanCommon::Device, &memoryAllocInfo, nullptr, &_vertexBufferMemory ), "failed to allocate memory" );
		CheckResult( vkBindBufferMemory( VulkanCommon::Device, _vertexBuffer, _vertexBufferMemory, 0 ), "failed to bind memory" );

		void *data;
		vkMapMemory( VulkanCommon::Device, _vertexBufferMemory, 0, vertexBufferCreateInfo.size, 0, &data );
		memcpy( data, bufferData, (size_t)vertexBufferCreateInfo.size );
		vkUnmapMemory( VulkanCommon::Device, _vertexBufferMemory );
	}

	void Vulkan::Draw()
	{
		uint32_t imageIndex = 0;
		CheckResult( vkAcquireNextImageKHR( VulkanCommon::Device, _swapchain._swapchain, 0ull, _deviceManager._semaphores[0].acquireComplete, VK_NULL_HANDLE, &imageIndex ), "failed to acquire next image" );

		VkPipelineStageFlags submitStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &_deviceManager._semaphores[0].acquireComplete;
		submitInfo.pWaitDstStageMask = &submitStageFlags;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &_commandBuffers[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &_deviceManager._semaphores[0].renderComplete;

		vkQueueSubmit( _deviceManager._gQueue, 1, &submitInfo, VK_NULL_HANDLE );

		VkPresentInfoKHR presentInfo;
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &_deviceManager._semaphores[0].renderComplete;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &_swapchain._swapchain;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		CheckResult( vkQueuePresentKHR( _deviceManager._gQueue, &presentInfo ), "failed to queue present" );
		CheckResult( vkDeviceWaitIdle( VulkanCommon::Device ), "failed to device wait idle" );
	}
}

