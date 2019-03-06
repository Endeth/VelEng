#include "vVulkan.h"
#include "vVulkanCommon.h"

//TEST DIRECTIVES
#include <chrono>

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
		CreateCommandBuffers();
		CreateStagingBuffer();
		CreateBuffer();
		CreateDepthImage();
		CreateImage();
		Samplers.CreateSamplers();
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSets();
		_renderPass.Create();
		_renderPass.CreatePipeline( _descriptorSetLayout, _pipelineLayout );
		_renderPass.CreateFramebuffers( _swapchain._images, _depthImage, _swapchain._imageSize );
		RecordCommandBuffers();
    }

    void Vulkan::Destroy()
    {
		//TODO check if commands are still being executed
		_vertexBuffer.Destroy();
		_stagingBuffer.Destroy();
		_indexBuffer.Destroy();
		Samplers.DestroySamplers();
		_sampledImage.Destroy();
		_depthImage.Destroy();

		for( auto &buffer : _uniformBuffers )
			buffer.Destroy();

		vkDestroyCommandPool( VulkanCommon::Device, _commandPoolGraphics, nullptr );
		vkDestroyCommandPool( VulkanCommon::Device, _commandPoolTransfer, nullptr );
		vkDestroyPipelineLayout( VulkanCommon::Device, _pipelineLayout, nullptr );
		_pipelineLayout = VK_NULL_HANDLE;
		_commandPoolTransfer = VK_NULL_HANDLE;
		_commandPoolGraphics = VK_NULL_HANDLE;
		vkDestroyDescriptorSetLayout( VulkanCommon::Device, _descriptorSetLayout, nullptr );
		_descriptorSetLayout = VK_NULL_HANDLE;
		vkDestroyDescriptorPool( VulkanCommon::Device, _descriptorPool, nullptr );
		_descriptorPool = VK_NULL_HANDLE;

		_swapchain.Cleanup();
		_renderPass.Cleanup();
		_deviceManager.Destroy();

#ifdef _DEBUG
		VulkanDebug::Instance()->DisableCallback();
#endif

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
		CreateCommandPool( 0, _deviceManager._queueFamilyIndices.graphics, &_commandPoolGraphics );
		CreateCommandPool( VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, _deviceManager._queueFamilyIndices.transfer, &_commandPoolTransfer );

		uint32_t imagesCount = _swapchain._images.size();

		_commandBuffers.resize( imagesCount, VK_NULL_HANDLE );

		AllocateCommandBuffers( imagesCount, _commandBuffers.data(), _commandPoolGraphics, VK_COMMAND_BUFFER_LEVEL_PRIMARY );
	}

	void Vulkan::RecordCommandBuffers()
	{
		VkCommandBufferBeginInfo beginInfo;
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		std::array<VkClearValue, 2> clearValues;
		clearValues[0].color = { 48.f / 256.f, 10 / 256.f, 36 / 256.f, 1.f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		VkRect2D renderArea;
		renderArea.offset = { 0, 0 };
		renderArea.extent = { static_cast<uint32_t>( VulkanOptions::WindowSize.x ), static_cast<uint32_t>( VulkanOptions::WindowSize.y ) };

		uint32_t imageCount = _swapchain._images.size();

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = _renderPass._renderPass;
		renderPassBeginInfo.renderArea = renderArea;
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		VkBuffer vertexBuffers[] = { _vertexBuffer._buffer };
		VkDeviceSize offsets[] = { 0 };

		VkImageSubresourceRange subresourceRange;
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

		/*
		VkImageMemoryBarrier barrierFromPresentToDraw;
		barrierFromPresentToDraw.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrierFromPresentToDraw.pNext = nullptr;
		barrierFromPresentToDraw.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrierFromPresentToDraw.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrierFromPresentToDraw.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrierFromPresentToDraw.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrierFromPresentToDraw.srcQueueFamilyIndex = 0;
		barrierFromPresentToDraw.dstQueueFamilyIndex = 0;
		barrierFromPresentToDraw.subresourceRange = subresourceRange;

		VkImageMemoryBarrier barrierFromDrawToPresent;
		barrierFromDrawToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrierFromDrawToPresent.pNext = nullptr;
		barrierFromDrawToPresent.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrierFromDrawToPresent.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrierFromDrawToPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrierFromDrawToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrierFromDrawToPresent.srcQueueFamilyIndex = 0;
		barrierFromDrawToPresent.dstQueueFamilyIndex = 0;
		barrierFromDrawToPresent.subresourceRange = subresourceRange; */


		for( int i = 0; i < imageCount; ++i )
		{
			renderPassBeginInfo.framebuffer = _renderPass._framebuffers[i]._framebuffer;
			//barrierFromPresentToDraw.image = _swapchain._images[i]._image; //TODO this is kind of a hack I guess
			//barrierFromDrawToPresent.image = _swapchain._images[i]._image; //TODO this is kind of a hack I guess

			CheckResult( vkBeginCommandBuffer( _commandBuffers[i], &beginInfo ), "failed to begin command buffer" );

			//vkCmdPipelineBarrier( _commandBuffers[i], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierFromPresentToDraw );
			vkCmdBeginRenderPass( _commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
			vkCmdBindPipeline( _commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _renderPass._graphicsPipeline );

			vkCmdBindVertexBuffers( _commandBuffers[i], 0, 1, vertexBuffers, offsets );
			vkCmdBindIndexBuffer( _commandBuffers[i], _indexBuffer._buffer, 0, VK_INDEX_TYPE_UINT32 );
			vkCmdBindDescriptorSets( _commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[i], 0, nullptr );

			vkCmdDrawIndexed( _commandBuffers[i], 12, 1, 0, 0, 0 );

			vkCmdEndRenderPass( _commandBuffers[i] );
			//vkCmdPipelineBarrier( _commandBuffers[i], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierFromDrawToPresent );

			CheckResult( vkEndCommandBuffer( _commandBuffers[i] ), "failed to end command buffer" );
		}
	}

	void Vulkan::CreateDescriptorPool()
	{
		std::array<VkDescriptorSetLayoutBinding, 2> descriptorSetsLayoutsBindings;
		descriptorSetsLayoutsBindings[0].binding = 0;
		descriptorSetsLayoutsBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorSetsLayoutsBindings[0].descriptorCount = 1;
		descriptorSetsLayoutsBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		descriptorSetsLayoutsBindings[0].pImmutableSamplers = nullptr;

		descriptorSetsLayoutsBindings[1].binding = 1;
		descriptorSetsLayoutsBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorSetsLayoutsBindings[1].descriptorCount = 1;
		descriptorSetsLayoutsBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptorSetsLayoutsBindings[1].pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
		descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCreateInfo.pNext = nullptr;
		descriptorSetLayoutCreateInfo.flags = 0;
		descriptorSetLayoutCreateInfo.bindingCount = 2;
		descriptorSetLayoutCreateInfo.pBindings = descriptorSetsLayoutsBindings.data();

		CheckResult( vkCreateDescriptorSetLayout( VulkanCommon::Device, &descriptorSetLayoutCreateInfo, nullptr, &_descriptorSetLayout ), "failed to create descriptor set" ); //TODO move this to a place where it can be reused

		std::array<VkDescriptorPoolSize, 2> descriptorPoolsSizes;
		descriptorPoolsSizes[0].descriptorCount = static_cast<uint32_t>( _swapchain._images.size() );
		descriptorPoolsSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorPoolsSizes[1].descriptorCount = static_cast<uint32_t>( _swapchain._images.size() );
		descriptorPoolsSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.pNext = nullptr;
		descriptorPoolCreateInfo.flags = 0;
		descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>( _swapchain._images.size() );
		descriptorPoolCreateInfo.poolSizeCount = descriptorPoolsSizes.size();
		descriptorPoolCreateInfo.pPoolSizes = descriptorPoolsSizes.data();

		CheckResult( vkCreateDescriptorPool( VulkanCommon::Device, &descriptorPoolCreateInfo, nullptr, &_descriptorPool ), "failed to create descriptor pool" );
	}

	void Vulkan::CreateDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts( _swapchain._images.size(), _descriptorSetLayout );
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = _descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>( _swapchain._images.size() );
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();

		_descriptorSets.resize( _swapchain._images.size() );
		CheckResult( vkAllocateDescriptorSets( VulkanCommon::Device, &descriptorSetAllocateInfo, _descriptorSets.data() ), "failed to create descriptor sets" );

		VkDescriptorBufferInfo descriptorBufferInfo;
		descriptorBufferInfo.offset = 0;
		descriptorBufferInfo.range = sizeof( CameraMatrices );

		VkDescriptorImageInfo descriptorImageInfo;
		descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptorImageInfo.imageView = _sampledImage._imageView;
		descriptorImageInfo.sampler = Samplers.GetSampler( VulkanSamplers::Type::BasicSampler );

		std::array<VkWriteDescriptorSet, 2> writeDescriptorSets;
		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].pNext = nullptr;
		writeDescriptorSets[0].dstBinding = 0;
		writeDescriptorSets[0].dstArrayElement = 0;
		writeDescriptorSets[0].descriptorCount = 1;
		writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[0].pImageInfo = nullptr;
		writeDescriptorSets[0].pBufferInfo = &descriptorBufferInfo;
		writeDescriptorSets[0].pTexelBufferView = nullptr;

		writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[1].pNext = nullptr;
		writeDescriptorSets[1].dstBinding = 1;
		writeDescriptorSets[1].dstSet = nullptr;
		writeDescriptorSets[1].dstArrayElement = 0;
		writeDescriptorSets[1].descriptorCount = 1;
		writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[1].pImageInfo = &descriptorImageInfo;
		writeDescriptorSets[1].pBufferInfo = nullptr;
		writeDescriptorSets[1].pTexelBufferView = nullptr;

		for( size_t i = 0; i < _swapchain._images.size(); ++i )
		{
			descriptorBufferInfo.buffer = _uniformBuffers[i]._buffer;
			writeDescriptorSets[0].dstSet = _descriptorSets[i];
			writeDescriptorSets[1].dstSet = _descriptorSets[i];

			vkUpdateDescriptorSets( VulkanCommon::Device, static_cast<uint32_t>( writeDescriptorSets.size() ), writeDescriptorSets.data(), 0, nullptr );
		}
	}

	void Vulkan::CreateStagingBuffer()
	{
		VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		std::vector<uint32_t> queueFamilyIndices( { _deviceManager._queueFamilyIndices.transfer, _deviceManager._queueFamilyIndices.graphics } );
		_stagingBuffer.CreateBuffer( 0, 1024 * 1024 * 4, usageFlags, VK_SHARING_MODE_CONCURRENT, queueFamilyIndices ); //TODO different size - current = size of 1024x1024 image with 4 channels
		auto memTypeIndex = _deviceManager._physicalDeviceProperties.FindMemoryType( _stagingBuffer._memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
		_stagingBuffer.AllocateMemory( memTypeIndex );
	}

	void Vulkan::CreateBuffer()
	{
		VertexUVColor bufferData[8] = {
			VertexUVColor( glm::vec3( -1.f, -1.f, 0.f ), glm::vec4( 1.f, 0.0f, 0.5f, 1.f ), glm::vec2{ 0.f, 0.f } ),
			VertexUVColor( glm::vec3( 1.f, 1.f, 0.f ), glm::vec4( 0.f, 0.5f, 1.f, 1.f ), glm::vec2{ 1.f, 1.f } ),
			VertexUVColor( glm::vec3( -1.f, 1.f, 0.f ), glm::vec4( 0.f, 1.f, 0.5f, 1.f ), glm::vec2{ 0.f, 1.f } ),
			VertexUVColor( glm::vec3( 1.f, -1.f, 0.f ), glm::vec4( 1.f, 0.5f, 0.f, 1.f ), glm::vec2{ 1.f, 0.f } ),

			VertexUVColor( glm::vec3( -1.5f, -1.5f, -2.f ), glm::vec4( 1.f, 0.0f, 0.0f, 1.f ), glm::vec2{ 0.f, 0.f } ),
			VertexUVColor( glm::vec3( 1.5f, 1.5f, -2.f ), glm::vec4( 0.f, 1.f, 0.f, 1.f ), glm::vec2{ 1.f, 1.f } ),
			VertexUVColor( glm::vec3( -1.5f, 1.5f, -2.f ), glm::vec4( 0.f, 0.f, 1.f, 1.f ), glm::vec2{ 0.f, 1.f } ),
			VertexUVColor( glm::vec3( 1.5f, -1.5f, -2.f ), glm::vec4( 1.f, 0.5f, 0.f, 1.f ), glm::vec2{ 1.f, 0.f } )
		};

		uint32_t indices[12] = {
			0, 1, 2, 3, 1, 0,
			4, 5, 6, 7, 5, 4
		};

		std::vector<uint32_t> queueFamilyIndices( { _deviceManager._queueFamilyIndices.transfer, _deviceManager._queueFamilyIndices.graphics } );
		VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		_vertexBuffer.CreateBuffer( 0, sizeof( bufferData ), usageFlags, VK_SHARING_MODE_CONCURRENT, queueFamilyIndices );
		auto memTypeIndex = _deviceManager._physicalDeviceProperties.FindMemoryType( _vertexBuffer._memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
		_vertexBuffer.AllocateMemory( memTypeIndex );

		_stagingBuffer.CopyDataToBuffer
		( bufferData, sizeof( bufferData ) );
		CopyBuffer( _stagingBuffer, _vertexBuffer, sizeof( bufferData ), _commandPoolTransfer, _deviceManager._tQueue );

		usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;;
		_indexBuffer.CreateBuffer( 0, sizeof( indices ), usageFlags, VK_SHARING_MODE_CONCURRENT, queueFamilyIndices );
		memTypeIndex = _deviceManager._physicalDeviceProperties.FindMemoryType( _indexBuffer._memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ); //TODO might move this to createbuffer
		_indexBuffer.AllocateMemory( memTypeIndex );

		_stagingBuffer.CopyDataToBuffer( indices, sizeof( indices ) );
		CopyBuffer( _stagingBuffer, _indexBuffer, sizeof( indices ), _commandPoolTransfer, _deviceManager._tQueue );
	}

	void Vulkan::CreateImage()
	{
		TexelData testImg( "assets/blood_moon.png" );
		_stagingBuffer.CopyDataToBuffer( testImg._data, testImg.GetSize() );

		std::vector<uint32_t> queueFamilyIndices( { _deviceManager._queueFamilyIndices.transfer, _deviceManager._queueFamilyIndices.graphics } );
		_sampledImage.Create( testImg._imageSize, testImg.GetSize(), VK_FORMAT_R8G8B8A8_UNORM, 0, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_CONCURRENT, queueFamilyIndices );

		auto memTypeIndex = _deviceManager._physicalDeviceProperties.FindMemoryType( _sampledImage._memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
		_sampledImage.AllocateMemory( memTypeIndex );

		_sampledImage.AdjustMemoryBarrier( VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _commandPoolTransfer, _deviceManager._tQueue );
		CopyImage( _stagingBuffer, _sampledImage, testImg.GetSize(), _commandPoolTransfer, _deviceManager._tQueue );
		_sampledImage.AdjustMemoryBarrier( VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, _commandPoolGraphics, _deviceManager._gQueue );
	}

	void Vulkan::CreateDepthImage()
	{
		std::vector<uint32_t> queueFamilyIndices( { _deviceManager._queueFamilyIndices.graphics } );
		VkDeviceSize deviceSize = _swapchain._imageSize.x * _swapchain._imageSize.y * 4;
		_depthImage.Create( _swapchain._imageSize, deviceSize, VK_FORMAT_D32_SFLOAT_S8_UINT, 0, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, queueFamilyIndices );

		auto memTypeIndex = _deviceManager._physicalDeviceProperties.FindMemoryType( _depthImage._memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
		_depthImage.AllocateMemory( memTypeIndex );

		_depthImage.AdjustMemoryBarrier( VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, _commandPoolGraphics, _deviceManager._gQueue );
	}

	void Vulkan::CreateUniformBuffers()
	{
		_uniformBuffers.resize( _swapchain._images.size() );
		
		VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		std::vector<uint32_t> queueFamilyIndices( { _deviceManager._queueFamilyIndices.transfer, _deviceManager._queueFamilyIndices.graphics } );
		
		for( auto &buffer : _uniformBuffers )
			buffer.CreateBuffer( 0, sizeof( CameraMatrices ), usageFlags, VK_SHARING_MODE_CONCURRENT, queueFamilyIndices );

		auto memTypeIndex = _deviceManager._physicalDeviceProperties.FindMemoryType( _uniformBuffers.front()._memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

		for( auto &buffer : _uniformBuffers )
			buffer.AllocateMemory( memTypeIndex );
	}

	void Vulkan::UpdateUniformBuffers( uint32_t imageIndex )
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>( currentTime - startTime ).count();

		_internalCamera.Model = glm::mat4( 1.f ); // glm::rotate( glm::mat4( 1.f ), time * glm::radians( 90.f ), glm::vec3( 0.f, 0.f, 1.f ) );
		
		_uniformBuffers[imageIndex].CopyDataToBuffer( &_internalCamera, sizeof( CameraMatrices ) );
	}

	void Vulkan::UpdateCamera( glm::mat4 & view, glm::mat4 & proj )
	{
		_internalCamera.View = view;
		_internalCamera.Projection = proj;
		_internalCamera.Projection[1][1] *= -1; //inverting Y (glm was designed for OpenGL)
	}

	void Vulkan::Draw()
	{
		uint32_t imageIndex = 0;
		CheckResult( vkAcquireNextImageKHR( VulkanCommon::Device, _swapchain._swapchain, 0ull, _deviceManager._semaphores[0].acquireComplete, VK_NULL_HANDLE, &imageIndex ), "failed to acquire next image" );

		UpdateUniformBuffers( imageIndex );
		
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
		CheckResult( vkDeviceWaitIdle( VulkanCommon::Device ), "failed to device wait idle" ); //TODO work on max 2 frames at the same time
	}
}

