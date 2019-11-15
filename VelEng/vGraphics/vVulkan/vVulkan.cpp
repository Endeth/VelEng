#include "vVulkan.h"
#include "vVulkanCommon.h"

//TEST DIRECTIVES
#include "vGraphics/vDrawable/vModel.h"
#include <chrono>
#include <unordered_map>

namespace Vel
{
    void VulkanRenderer::Init( GLFWwindow *window )
    {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
        appInfo.pApplicationName = "Vulkan";
        appInfo.applicationVersion = VK_MAKE_VERSION( 0, 1, 0 );
        appInfo.pEngineName = "VelEng";
        appInfo.engineVersion = VK_MAKE_VERSION( 0, 1, 0 );
        appInfo.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext = nullptr;
        createInfo.pApplicationInfo = &appInfo;
		createInfo.flags = 0;

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

		VulkanCommon::DeviceManager.Setup();

		CreateSwapchain( window );
		CreateCommandBuffers();
		CreateStagingBuffer(); //TODO figure out buffers
		LoadOBJ(); //TODO instead of creating temporary obj for testing create scene which will load objects with lua
		CreateBuffer();
		CreateDepthImage();
		CreateImage(); //TODO as with LoadOBJ
		Samplers.CreateSamplers();
		CreateUniformBuffers(); //TODO uniform buffers per shader/image?
		CreateDescriptorPool();
		CreateDescriptorSets();
		renderPass.Create();
		//_testingMaterial = std::make_shared<Material>( /*std::string( "assets/cube.obj" )*/ ); //TODO decide whether materialpipeline makes sense
		//_testingMaterial->pipeline.Create( renderPass.GetRenderPass(), descriptorSetLayout );
		//renderPass.CreatePipeline( descriptorSetLayout, pipelineLayout ); //TODO create defined pipelines - define sample material with sample pipeline
		renderPass.CreateFrameContexts( swapchain.images, depthImage, swapchain.imageSize );
		RecordCommandBuffers();
    }

    void VulkanRenderer::Destroy()
    {
		//TODO check if commands are still being executed
		vertexBuffer.Destroy();
		stagingBuffer.Destroy();
		indexBuffer.Destroy();
		Samplers.DestroySamplers();
		sampledImage.Destroy();
		depthImage.Destroy();

		for( auto &buffer : uniformBuffers )
			buffer.Destroy();

		vkDestroyCommandPool( VulkanCommon::Device, commandPoolGraphics, nullptr );
		vkDestroyCommandPool( VulkanCommon::Device, commandPoolTransfer, nullptr );
		commandPoolTransfer = VK_NULL_HANDLE;
		commandPoolGraphics = VK_NULL_HANDLE;
		vkDestroyDescriptorPool( VulkanCommon::Device, descriptorPool, nullptr );
		descriptorPool = VK_NULL_HANDLE;

		swapchain.Cleanup();
		renderPass.Cleanup();
		VulkanCommon::DeviceManager.Destroy();

#ifdef _DEBUG
		VulkanDebug::Instance()->DisableCallback();
#endif

        vkDestroyInstance( VulkanCommon::Instance, nullptr );
		VulkanCommon::Instance = VK_NULL_HANDLE;
    }

	void VulkanRenderer::CreateSwapchain( GLFWwindow *window )
	{
		swapchain.CreateSurface( window );

		VkBool32 presentSupported = 0; //Create Proper Present Queue
		vkGetPhysicalDeviceSurfaceSupportKHR( VulkanCommon::PhysicalDevice, VulkanCommon::DeviceManager.queueFamilyIndices.graphics, swapchain.surface, &presentSupported );

		VulkanCommon::DeviceManager.physicalDeviceProperties.QuerySwapchainSupport( swapchain.surface );
		swapchain.CreateSwapchain( VulkanCommon::DeviceManager.physicalDeviceProperties.swapchainSupport, VulkanCommon::DeviceManager.queueFamilyIndices.graphics );
	}

	void VulkanRenderer::CreateCommandBuffers()
	{
		CreateCommandPool( 0, VulkanCommon::DeviceManager.queueFamilyIndices.graphics, &commandPoolGraphics );
		CreateCommandPool( VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, VulkanCommon::DeviceManager.queueFamilyIndices.transfer, &commandPoolTransfer );

		size_t imagesCount = swapchain.images.size();

		commandBuffers.resize( imagesCount, VK_NULL_HANDLE );

		AllocateCommandBuffers( imagesCount, commandBuffers.data(), commandPoolGraphics, VK_COMMAND_BUFFER_LEVEL_PRIMARY );
	}

	void VulkanRenderer::RecordCommandBuffers()
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

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = renderPass.GetRenderPass(); //TODO
		renderPassBeginInfo.renderArea = renderArea;
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		VkBuffer vertexBuffers[] = { vertexBuffer._buffer };
		VkDeviceSize offsets[] = { 0 };

		VkImageSubresourceRange subresourceRange;
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

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
		barrierFromDrawToPresent.subresourceRange = subresourceRange;


		size_t swapchainImagesCount = swapchain.images.size();
		for( int imageId = 0; imageId < swapchainImagesCount; ++imageId )
		{
			auto& frameContext = renderPass.GetFrameContext( imageId );
			VkCommandBuffer frameCmdBuffer = frameContext.GetCommandBuffer();
			renderPassBeginInfo.framebuffer = frameContext.GetFramebuffer();
			barrierFromPresentToDraw.image = swapchain.images[i].image; //TODO this is kind of a hack I guess
			barrierFromDrawToPresent.image = swapchain.images[i].image; //TODO this is kind of a hack I guess

			CheckResult( vkBeginCommandBuffer( frameCmdBuffer, &beginInfo ), "failed to begin command buffer" );

			vkCmdPipelineBarrier(frameCmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierFromPresentToDraw );
			vkCmdBeginRenderPass( frameCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

			vkCmdBindPipeline( frameCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass._graphicsPipeline ); //TODO

			vkCmdBindVertexBuffers( frameCmdBuffer, 0, 1, vertexBuffers, offsets );
			vkCmdBindIndexBuffer( frameCmdBuffer, indexBuffer._buffer, 0, VK_INDEX_TYPE_UINT32 );
			vkCmdBindDescriptorSets( frameCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr ); //TODO

			vkCmdDrawIndexed( frameCmdBuffer, _testingModel->_meshes[0]->GetIndicesCount(), 1, 0, 0, 0 );

			vkCmdEndRenderPass( frameCmdBuffer );
			vkCmdPipelineBarrier( frameCmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierFromDrawToPresent );

			CheckResult( vkEndCommandBuffer( frameCmdBuffer ), "failed to end command buffer" );
		}
	}

	void VulkanRenderer::CreateDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, descriptorSetsLayoutsBindings.size()> descriptorPoolsSizes;
		descriptorPoolsSizes[0].descriptorCount = static_cast<uint32_t>( swapchain.images.size() );
		descriptorPoolsSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorPoolsSizes[1].descriptorCount = static_cast<uint32_t>( swapchain.images.size() ); //TODO
		descriptorPoolsSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorPoolsSizes[2].descriptorCount = static_cast<uint32_t>( swapchain.images.size() );
		descriptorPoolsSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.pNext = nullptr;
		descriptorPoolCreateInfo.flags = 0;
		descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>( swapchain.images.size() );
		descriptorPoolCreateInfo.poolSizeCount = descriptorPoolsSizes.size();
		descriptorPoolCreateInfo.pPoolSizes = descriptorPoolsSizes.data();

		CheckResult( vkCreateDescriptorPool( VulkanCommon::Device, &descriptorPoolCreateInfo, nullptr, &descriptorPool ), "failed to create descriptor pool" );
	}

	void VulkanRenderer::CreateDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts( swapchain.images.size(), descriptorSetLayout );
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>( swapchain.images.size() );
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();

		descriptorSets.resize( swapchain.images.size() );
		CheckResult( vkAllocateDescriptorSets( VulkanCommon::Device, &descriptorSetAllocateInfo, descriptorSets.data() ), "failed to create descriptor sets" );

		VkDescriptorBufferInfo descriptorBufferInfo;
		descriptorBufferInfo.offset = 0;
		descriptorBufferInfo.range = sizeof( CameraMatrices ); //TODO i dont think 0 and 1 both need whole cameramatrices.range

		VkDescriptorImageInfo descriptorImageInfo;
		descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptorImageInfo.imageView = sampledImage.imageView;
		descriptorImageInfo.sampler = Samplers.GetSampler( VulkanSamplers::Type::BasicSampler );

		std::array<VkWriteDescriptorSet, 3> writeDescriptorSets;
		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].pNext = nullptr;
		writeDescriptorSets[0].dstBinding = 0;
		writeDescriptorSets[0].dstSet = nullptr;
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
		writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[1].pImageInfo = nullptr;
		writeDescriptorSets[1].pBufferInfo = &descriptorBufferInfo;
		writeDescriptorSets[1].pTexelBufferView = nullptr;

		writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[2].pNext = nullptr;
		writeDescriptorSets[2].dstBinding = 2;
		writeDescriptorSets[2].dstSet = nullptr;
		writeDescriptorSets[2].dstArrayElement = 0;
		writeDescriptorSets[2].descriptorCount = 1;
		writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[2].pImageInfo = &descriptorImageInfo;
		writeDescriptorSets[2].pBufferInfo = nullptr;
		writeDescriptorSets[2].pTexelBufferView = nullptr;

		for( size_t i = 0; i < swapchain.images.size(); ++i )
		{
			descriptorBufferInfo.buffer = uniformBuffers[i]._buffer;
			writeDescriptorSets[0].dstSet = descriptorSets[i];
			writeDescriptorSets[1].dstSet = descriptorSets[i];
			writeDescriptorSets[2].dstSet = descriptorSets[i];

			vkUpdateDescriptorSets( VulkanCommon::Device, static_cast<uint32_t>( writeDescriptorSets.size() ), writeDescriptorSets.data(), 0, nullptr );
		}
	}

	void VulkanRenderer::LoadOBJ()
	{
		_testingModel = std::make_shared<Model>( std::string( "assets/cube.obj" ) );
		/*tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		//std::string modelPath( "assets/chalet.obj" );

		std::unordered_map<VertexUVColor, uint32_t> uniqueVertices = {};

		if( !tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, "assets/chalet.obj" ) )
		{
			throw std::runtime_error( warn + err );
		}
		 
		for( const auto& shape : shapes )
		{
			for( const auto& index : shape.mesh.indices )
			{
				VertexUVColor vertex;
				vertex.position = glm::vec3( attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2] );
				vertex.UV = glm::vec2( attrib.texcoords[2 * index.texcoord_index + 0], attrib.texcoords[2 * index.texcoord_index + 1] );
				vertex.color = glm::vec4( 1.f, 1.f, 1.f, 1.f );

				if( uniqueVertices[vertex] == 0 )
				{
					uniqueVertices[vertex] = static_cast<uint32_t>( _vertices.size() );
					_vertices.push_back( vertex );
				}

				_indices.push_back( _indices.size() );
			}
		}*/
	}

	void VulkanRenderer::CreateStagingBuffer()
	{
		VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		std::vector<uint32_t> queueFamilyIndices( { VulkanCommon::DeviceManager.queueFamilyIndices.transfer, VulkanCommon::DeviceManager.queueFamilyIndices.graphics } );
		stagingBuffer.CreateBuffer( 0, 4096 * 4096 * 4, usageFlags, VK_SHARING_MODE_CONCURRENT, queueFamilyIndices ); //TODO different size - current = size of 4096x4096 image with 4 channels
		auto memTypeIndex = VulkanCommon::DeviceManager.physicalDeviceProperties.FindMemoryType( stagingBuffer.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ); //For debugging flush may be better
		stagingBuffer.AllocateMemory( memTypeIndex );
	}

	void VulkanRenderer::CreateBuffer()
	{
		auto testMesh = _testingModel->_meshes[0]; //TODO UGLY TESTS TESTS TESTS
		auto modelSize = sizeof( VertexUVColor ) * testMesh->GetVerticesSize();
		auto indicesSize = sizeof( unsigned int ) * testMesh->GetIndicesSize();

		VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		std::vector<uint32_t> queueFamilyIndices( { VulkanCommon::DeviceManager.queueFamilyIndices.transfer, VulkanCommon::DeviceManager.queueFamilyIndices.graphics } );
		VulkanBuffer modelStagingBuffer;
		modelStagingBuffer.CreateBuffer( 0, modelSize, usageFlags, VK_SHARING_MODE_CONCURRENT, queueFamilyIndices ); //TODO different size - current = size of 4096x4096 image with 4 channels
		auto memTypeIndex = VulkanCommon::DeviceManager.physicalDeviceProperties.FindMemoryType( modelStagingBuffer.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
		modelStagingBuffer.AllocateMemory( memTypeIndex );

		usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		vertexBuffer.CreateBuffer( 0, modelSize, usageFlags, VK_SHARING_MODE_CONCURRENT, queueFamilyIndices );
		memTypeIndex = VulkanCommon::DeviceManager.physicalDeviceProperties.FindMemoryType( vertexBuffer.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
		vertexBuffer.AllocateMemory( memTypeIndex );

		modelStagingBuffer.CopyDataToBuffer( testMesh->_vertices.data(), modelSize );
		CopyBuffer( modelStagingBuffer, vertexBuffer, modelSize, commandPoolTransfer, VulkanCommon::DeviceManager.tQueue );

		usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;;
		indexBuffer.CreateBuffer( 0, indicesSize, usageFlags, VK_SHARING_MODE_CONCURRENT, queueFamilyIndices );
		memTypeIndex = VulkanCommon::DeviceManager.physicalDeviceProperties.FindMemoryType( indexBuffer.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ); //TODO might move this to createbuffer
		indexBuffer.AllocateMemory( memTypeIndex );

		modelStagingBuffer.CopyDataToBuffer( testMesh->_indices.data(), indicesSize );
		CopyBuffer( modelStagingBuffer, indexBuffer, indicesSize, commandPoolTransfer, VulkanCommon::DeviceManager.tQueue );

		modelStagingBuffer.Destroy();
	}

	void VulkanRenderer::CreateImage()
	{
		TexelData testImg( "assets/chalet.png" ); //TODO check for errors during loading
		stagingBuffer.CopyDataToBuffer( testImg.data, testImg.GetSize() );
		 
		std::vector<uint32_t> queueFamilyIndices( { VulkanCommon::DeviceManager.queueFamilyIndices.transfer, VulkanCommon::DeviceManager.queueFamilyIndices.graphics } );
		sampledImage.Create( testImg.imageSize, testImg.GetSize(), VK_FORMAT_R8G8B8A8_UNORM, 0, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_CONCURRENT, queueFamilyIndices );

		auto memTypeIndex = VulkanCommon::DeviceManager.physicalDeviceProperties.FindMemoryType( sampledImage.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
		sampledImage.AllocateMemory( memTypeIndex );

		sampledImage.AdjustMemoryBarrier( VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandPoolTransfer, VulkanCommon::DeviceManager.tQueue );
		CopyImage( stagingBuffer, sampledImage, testImg.GetSize(), commandPoolTransfer, VulkanCommon::DeviceManager.tQueue );
		sampledImage.AdjustMemoryBarrier( VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandPoolGraphics, VulkanCommon::DeviceManager.gQueue );
	}

	void VulkanRenderer::CreateDepthImage()
	{
		std::vector<uint32_t> queueFamilyIndices( { VulkanCommon::DeviceManager.queueFamilyIndices.graphics } );
		VkDeviceSize deviceSize = swapchain.imageSize.x * swapchain.imageSize.y * 4;
		depthImage.Create( swapchain.imageSize, deviceSize, VK_FORMAT_D32_SFLOAT_S8_UINT, 0, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, queueFamilyIndices );

		auto memTypeIndex = VulkanCommon::DeviceManager.physicalDeviceProperties.FindMemoryType( depthImage.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
		depthImage.AllocateMemory( memTypeIndex );

		depthImage.AdjustMemoryBarrier( VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, commandPoolGraphics, VulkanCommon::DeviceManager.gQueue );
	}

	void VulkanRenderer::CreateUniformBuffers()
	{
		uniformBuffers.resize( swapchain.images.size() );
		
		VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		std::vector<uint32_t> queueFamilyIndices( { VulkanCommon::DeviceManager.queueFamilyIndices.transfer, VulkanCommon::DeviceManager.queueFamilyIndices.graphics } );
		
		for( auto &buffer : uniformBuffers )
			buffer.CreateBuffer( 0, sizeof( CameraMatrices ), usageFlags, VK_SHARING_MODE_CONCURRENT, queueFamilyIndices );

		auto memTypeIndex = VulkanCommon::DeviceManager.physicalDeviceProperties.FindMemoryType( uniformBuffers.front().memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

		for( auto &buffer : uniformBuffers )
			buffer.AllocateMemory( memTypeIndex );
	}

	void VulkanRenderer::UpdateUniformBuffers( uint32_t imageIndex )
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>( currentTime - startTime ).count();

		internalCamera.Model = glm::mat4( 1.f ); // glm::rotate( glm::mat4( 1.f ), time * glm::radians( 90.f ), glm::vec3( 0.f, 0.f, 1.f ) );
		
		uniformBuffers[imageIndex].CopyDataToBuffer( &internalCamera, sizeof( CameraMatrices ) );
	}

	void VulkanRenderer::UpdateCamera( glm::mat4 & view, glm::mat4 & proj )
	{
		internalCamera.View = view;
		internalCamera.Projection = proj;
		internalCamera.Projection[1][1] *= -1; //inverting Y (glm was designed for OpenGL)
	}

	void VulkanRenderer::Draw()
	{
		uint32_t swapchainImageIndex = 0;
		CheckResult( swapchain.AcquireNextImage( &swapchainImageIndex, VulkanCommon::DeviceManager.semaphores.acquireComplete, VK_NULL_HANDLE ), "failed to acquire next image" );
		auto& frameContext = renderPass.GetFrameContext( swapchainImageIndex );
		UpdateUniformBuffers( swapchainImageIndex ); //TODO move this kind of uniforms to frame context
		VkCommandBuffer cmdbuffer = frameContext.GetCommandBuffer();
		VkPipelineStageFlags submitStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &VulkanCommon::DeviceManager.semaphores.acquireComplete;
		submitInfo.pWaitDstStageMask = &submitStageFlags;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdbuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &VulkanCommon::DeviceManager.semaphores.renderComplete;

		vkQueueSubmit( VulkanCommon::DeviceManager.gQueue, 1, &submitInfo, VK_NULL_HANDLE );

		VkPresentInfoKHR presentInfo;
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &VulkanCommon::DeviceManager.semaphores.renderComplete;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain.swapchain;
		presentInfo.pImageIndices = &swapchainImageIndex;
		presentInfo.pResults = nullptr;

		CheckResult( vkQueuePresentKHR( VulkanCommon::DeviceManager.gQueue, &presentInfo ), "failed to queue present" );
		CheckResult( vkDeviceWaitIdle( VulkanCommon::Device ), "failed to device wait idle" ); //TODO work on max 2 frames at the same time
	}
}

