#include <fstream>

#include "vVulkanRenderPass.h"

namespace Vel
{
	void VulkanRenderPass::Create()
	{
		VkAttachmentDescription attachementDesc;
		attachementDesc.flags = 0;
		attachementDesc.format = VK_FORMAT_B8G8R8A8_UNORM; //TODO use somekind of getter from swapchain
		attachementDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		attachementDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachementDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachementDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachementDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachementDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachementDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachementReference;
		colorAttachementReference.attachment = 0;
		colorAttachementReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription;
		subpassDescription.flags = 0;
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorAttachementReference;
		subpassDescription.pResolveAttachments = nullptr;
		subpassDescription.pDepthStencilAttachment = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;

		VkRenderPassCreateInfo renderPassCreateInfo = {};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.pNext = nullptr;
		renderPassCreateInfo.flags = 0;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &attachementDesc;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		renderPassCreateInfo.dependencyCount = 0;
		renderPassCreateInfo.pDependencies = nullptr;

		CheckResult( vkCreateRenderPass( VulkanCommon::Device, &renderPassCreateInfo, nullptr, &_renderPass ), "failed to create renderpass" );
	}

	void VulkanRenderPass::CreatePipeline( VkDescriptorSetLayout dscSetLayout, VkPipelineLayout &_pipelineLayout )
	{
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo;
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		pipelineCacheCreateInfo.pNext = nullptr;
		pipelineCacheCreateInfo.flags = 0;
		pipelineCacheCreateInfo.initialDataSize = 0;
		pipelineCacheCreateInfo.pInitialData = nullptr;

		vkCreatePipelineCache( VulkanCommon::Device, &pipelineCacheCreateInfo, nullptr, &_pipelineCache );

		VkShaderModule vertexShaderModule = CreateShaderModule( "shaders/shader.vert.spv" );
		VkShaderModule fragmentShaderModule = CreateShaderModule( "shaders/shader.frag.spv" );

		VkPipelineShaderStageCreateInfo shaderStageInfo[2] =
		{
			{
				VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				nullptr,
				0,
				VK_SHADER_STAGE_VERTEX_BIT,
				vertexShaderModule,
				"main",
				nullptr
			},
			{
				VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				nullptr,
				0,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				fragmentShaderModule,
				"main",
				nullptr
			}
		};

		VkVertexInputBindingDescription vertexInputBindingDescription;
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof(VertexColor);
		vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription vertexInputAttributeDescription[2];
		vertexInputAttributeDescription[0].location = 0;
		vertexInputAttributeDescription[0].binding = 0;
		vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescription[0].offset = offsetof( struct VertexColor, position.x );
		vertexInputAttributeDescription[1].location = 1;
		vertexInputAttributeDescription[1].binding = 0;
		vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		vertexInputAttributeDescription[1].offset = offsetof( struct VertexColor, color.r );

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo;
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateInfo.pNext = nullptr;
		vertexInputStateInfo.flags = 0;
		vertexInputStateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
		vertexInputStateInfo.vertexAttributeDescriptionCount = 2;
		vertexInputStateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescription;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo;
		inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyStateInfo.pNext = nullptr;
		inputAssemblyStateInfo.flags = 0;
		inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyStateInfo.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport; //TODO size of surface
		viewport.x = 0.f;
		viewport.y = 0.f;
		viewport.width = static_cast<float>( VulkanOptions::WindowSize.x );
		viewport.height = static_cast<float>( VulkanOptions::WindowSize.y );
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		VkRect2D scissor;
		scissor.offset = { 0, 0 };
		scissor.extent = { static_cast<uint32_t>( VulkanOptions::WindowSize.x ), static_cast<uint32_t>( VulkanOptions::WindowSize.y ) }; //TODO size of surface

		VkPipelineViewportStateCreateInfo viewportStateInfo;
		viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateInfo.pNext = nullptr;
		viewportStateInfo.flags = 0;
		viewportStateInfo.viewportCount = 1;
		viewportStateInfo.pViewports = &viewport;
		viewportStateInfo.scissorCount = 1;
		viewportStateInfo.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizationStateInfo;
		rasterizationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateInfo.pNext = nullptr;
		rasterizationStateInfo.flags = 0;
		rasterizationStateInfo.depthClampEnable = VK_FALSE;
		rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationStateInfo.depthBiasEnable = VK_FALSE;
		rasterizationStateInfo.depthBiasConstantFactor = 0.f;
		rasterizationStateInfo.depthBiasClamp = 0.f;
		rasterizationStateInfo.depthBiasSlopeFactor = 0.f;
		rasterizationStateInfo.lineWidth = 1.f;

		VkPipelineMultisampleStateCreateInfo multisampleStateInfo;
		multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateInfo.pNext = nullptr;
		multisampleStateInfo.flags = 0;
		multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleStateInfo.sampleShadingEnable = VK_FALSE;
		multisampleStateInfo.minSampleShading = 0.f;
		multisampleStateInfo.pSampleMask = nullptr;
		multisampleStateInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleStateInfo.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
		colorBlendAttachmentState.blendEnable = VK_FALSE;
		colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo colorBlendStateInfo;
		colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateInfo.pNext = nullptr;
		colorBlendStateInfo.flags = 0;
		colorBlendStateInfo.logicOpEnable = VK_FALSE;
		colorBlendStateInfo.logicOp = VK_LOGIC_OP_NO_OP;
		colorBlendStateInfo.attachmentCount = 1;
		colorBlendStateInfo.pAttachments = &colorBlendAttachmentState;
		colorBlendStateInfo.blendConstants[0] = 0.f;
		colorBlendStateInfo.blendConstants[1] = 0.f;
		colorBlendStateInfo.blendConstants[2] = 0.f;
		colorBlendStateInfo.blendConstants[3] = 0.f;

		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.pNext = nullptr;
		dynamicStateInfo.flags = 0;
		dynamicStateInfo.dynamicStateCount = 0;
		dynamicStateInfo.pDynamicStates = nullptr;

		VkPipelineLayout pipelineLayout;

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.flags = 0;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &dscSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

		CheckResult( vkCreatePipelineLayout( VulkanCommon::Device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout ), "failed to create pipeline layout" );
		_pipelineLayout = pipelineLayout;

		VkGraphicsPipelineCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.stageCount = 2;
		createInfo.pStages = shaderStageInfo; //TODO add shader modules here
		createInfo.pVertexInputState = &vertexInputStateInfo;
		createInfo.pInputAssemblyState = &inputAssemblyStateInfo;
		createInfo.pTessellationState = nullptr;
		createInfo.pViewportState = &viewportStateInfo;
		createInfo.pRasterizationState = &rasterizationStateInfo;
		createInfo.pMultisampleState = &multisampleStateInfo;
		createInfo.pDepthStencilState = nullptr; // &depthStencilStateInfo;
		createInfo.pColorBlendState = &colorBlendStateInfo;
		createInfo.pDynamicState = &dynamicStateInfo;
		createInfo.layout = pipelineLayout;
		createInfo.renderPass = _renderPass;
		createInfo.subpass = 0;
		createInfo.basePipelineHandle = VK_NULL_HANDLE;
		createInfo.basePipelineIndex = -1;

		vkCreateGraphicsPipelines( VulkanCommon::Device, _pipelineCache, 1, &createInfo, nullptr, &_graphicsPipeline );
		vkDestroyShaderModule( VulkanCommon::Device, vertexShaderModule, nullptr );
		vkDestroyShaderModule( VulkanCommon::Device, fragmentShaderModule, nullptr );
		vertexShaderModule = VK_NULL_HANDLE;
		fragmentShaderModule = VK_NULL_HANDLE;
	}

	void VulkanRenderPass::CreateFramebuffers( const std::vector<VulkanImage> &images, glm::i32vec2 size )
	{
		for( auto &image : images )
			_framebuffers.push_back( VulkanFramebuffer( _renderPass, std::vector<VulkanImage>( { image } ), size ) ); //TODO make this hack right, maybe func that gets all VkImages from VulkanImage[]
	}


	void VulkanRenderPass::Cleanup()
	{
		for( auto &framebuffer : _framebuffers )
			framebuffer.Cleanup();

		//vkFreeDescriptorSets( VulkanCommon::Device, _descriptorSet, nullptr );
		vkDestroyPipeline( VulkanCommon::Device, _graphicsPipeline, nullptr );
		vkDestroyPipelineCache( VulkanCommon::Device, _pipelineCache, nullptr );
		vkDestroyRenderPass( VulkanCommon::Device, _renderPass, nullptr );
		_descriptorSet = VK_NULL_HANDLE;
		_graphicsPipeline = VK_NULL_HANDLE;
		_pipelineCache = VK_NULL_HANDLE;
		_renderPass = VK_NULL_HANDLE;
	}

	VkShaderModule VulkanRenderPass::CreateShaderModule( const char *filepath )
	{
		VkShaderModule shader = VK_NULL_HANDLE;
		std::ifstream file( filepath, std::ios::binary );
		if( !file.fail() ) //TODO add fail
		{
			std::streampos begin, end;
			begin = file.tellg();
			file.seekg( 0, std::ios::end );
			end = file.tellg();

			std::vector<char> shaderCode( static_cast<size_t>( end - begin ) );
			file.seekg( 0, std::ios::beg );
			file.read( shaderCode.data(), end - begin );
			file.close();

			if( shaderCode.size() > 0 )
			{
				VkShaderModuleCreateInfo shaderModuleCreateInfo;
				shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				shaderModuleCreateInfo.pNext = nullptr;
				shaderModuleCreateInfo.flags = 0;
				shaderModuleCreateInfo.codeSize = shaderCode.size();
				shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>( shaderCode.data() );

				CheckResult( vkCreateShaderModule( VulkanCommon::Device, &shaderModuleCreateInfo, nullptr, &shader ), "failed to create shader" );
			}
		}

		return shader;
	}

	VulkanFramebuffer::VulkanFramebuffer( VkRenderPass renderPass, const std::vector<VulkanImage> &images, glm::u32vec2 size )
	{
		std::vector<VkImageView> attachements;
		for( auto &img : images )
		{
			attachements.push_back( img.ImageView );
		}

		VkFramebufferCreateInfo framebufferCreateInfo;
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext = nullptr;
		framebufferCreateInfo.flags = 0;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = attachements.size();
		framebufferCreateInfo.pAttachments = attachements.data();
		framebufferCreateInfo.width = size.x;
		framebufferCreateInfo.height = size.y;
		framebufferCreateInfo.layers = 1;

		CheckResult( vkCreateFramebuffer( VulkanCommon::Device, &framebufferCreateInfo, nullptr, &_framebuffer ), "failed to create framebuffer" );
	}

	void VulkanFramebuffer::Cleanup()
	{
		if( _framebuffer != VK_NULL_HANDLE )
			vkDestroyFramebuffer( VulkanCommon::Device, _framebuffer, nullptr );
	}
}