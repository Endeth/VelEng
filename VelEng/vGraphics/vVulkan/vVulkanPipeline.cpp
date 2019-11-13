#include <fstream>

#include "vVulkanPipeline.h"

namespace Vel
{
	void Pipeline::Create( VkRenderPass renderPass, VkDescriptorSetLayout dscSetLayout )
	{
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo;
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		pipelineCacheCreateInfo.pNext = nullptr;
		pipelineCacheCreateInfo.flags = 0;
		pipelineCacheCreateInfo.initialDataSize = 0;
		pipelineCacheCreateInfo.pInitialData = nullptr;

		//vkCreatePipelineCache( VulkanCommon::Device, &pipelineCacheCreateInfo, nullptr, &_pipelineCache ); //TODO add pipeline cache

		VkShaderModule vertexShaderModule = CreateShaderModule( "shaders/shader.vert.spv" ); //TODO proper shader loading
		VkShaderModule fragmentShaderModule = CreateShaderModule( "shaders/shader.frag.spv" );

		CreatePipelineLayout( dscSetLayout );
		CreatePipeline( renderPass, vertexShaderModule, fragmentShaderModule );

		vkDestroyShaderModule( VulkanCommon::Device, vertexShaderModule, nullptr );
		vkDestroyShaderModule( VulkanCommon::Device, fragmentShaderModule, nullptr );
		vertexShaderModule = VK_NULL_HANDLE;
		fragmentShaderModule = VK_NULL_HANDLE;
	}

	void Pipeline::CreatePipelineLayout( VkDescriptorSetLayout dscSetLayout )
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.flags = 0;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &dscSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

		CheckResult( vkCreatePipelineLayout( VulkanCommon::Device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout ), "failed to create pipeline layout" );
	}

	void Pipeline::CreatePipeline( VkRenderPass renderPass, VkShaderModule vertex, VkShaderModule fragment )
	{
		VkPipelineShaderStageCreateInfo shaderStageInfo[2] =
		{
			{
				VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				nullptr,
				0,
				VK_SHADER_STAGE_VERTEX_BIT,
				vertex,
				"main",
				nullptr
			},
			{
				VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				nullptr,
				0,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				fragment,
				"main",
				nullptr
			}
		};

		VkVertexInputBindingDescription vertexInputBindingDescription;
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof( VertexUVColor );
		vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VertexUVColor testVert;
		auto test = offsetof( struct VertexUVColor, color );

		VkVertexInputAttributeDescription vertexInputAttributeDescription[3]; //TODO different vertex types
		vertexInputAttributeDescription[0].location = 0;
		vertexInputAttributeDescription[0].binding = 0;
		vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescription[0].offset = offsetof( struct VertexUVColor, position );
		vertexInputAttributeDescription[1].location = 1;
		vertexInputAttributeDescription[1].binding = 0;
		vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		vertexInputAttributeDescription[1].offset = offsetof( struct VertexUVColor, color );
		vertexInputAttributeDescription[2].location = 2;
		vertexInputAttributeDescription[2].binding = 0;
		vertexInputAttributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributeDescription[2].offset = offsetof( struct VertexUVColor, UV );

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo;
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateInfo.pNext = nullptr;
		vertexInputStateInfo.flags = 0;
		vertexInputStateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
		vertexInputStateInfo.vertexAttributeDescriptionCount = 3;
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

		VkPipelineMultisampleStateCreateInfo multisampleStateInfo; //TODO multisampling
		multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateInfo.pNext = nullptr;
		multisampleStateInfo.flags = 0;
		multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleStateInfo.sampleShadingEnable = VK_FALSE;
		multisampleStateInfo.minSampleShading = 0.f;
		multisampleStateInfo.pSampleMask = nullptr;
		multisampleStateInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleStateInfo.alphaToOneEnable = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo;
		depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilStateInfo.pNext = nullptr;
		depthStencilStateInfo.flags = 0;
		depthStencilStateInfo.depthTestEnable = VK_TRUE;
		depthStencilStateInfo.depthWriteEnable = VK_TRUE;
		depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilStateInfo.minDepthBounds = 0.f;
		depthStencilStateInfo.maxDepthBounds = 1.f;
		depthStencilStateInfo.stencilTestEnable = VK_FALSE;
		depthStencilStateInfo.front = {};
		depthStencilStateInfo.back = {};

		VkPipelineColorBlendAttachmentState colorBlendAttachmentState; //TODO in far future
		colorBlendAttachmentState.blendEnable = VK_FALSE;
		colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo colorBlendStateInfo; //TODO in far future
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

		VkGraphicsPipelineCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.stageCount = 2;
		createInfo.pStages = shaderStageInfo;
		createInfo.pVertexInputState = &vertexInputStateInfo;
		createInfo.pInputAssemblyState = &inputAssemblyStateInfo;
		createInfo.pTessellationState = nullptr;
		createInfo.pViewportState = &viewportStateInfo;
		createInfo.pRasterizationState = &rasterizationStateInfo;
		createInfo.pMultisampleState = &multisampleStateInfo;
		createInfo.pDepthStencilState = &depthStencilStateInfo;
		createInfo.pColorBlendState = &colorBlendStateInfo;
		createInfo.pDynamicState = &dynamicStateInfo;
		createInfo.layout = pipelineLayout;
		createInfo.renderPass = renderPass;
		createInfo.subpass = 0;
		createInfo.basePipelineHandle = VK_NULL_HANDLE;
		createInfo.basePipelineIndex = -1;

		vkCreateGraphicsPipelines( VulkanCommon::Device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline ); //TODO pipeline cache
	}

	void Pipeline::Cleanup()
	{
		vkDestroyPipelineLayout( VulkanCommon::Device, pipelineLayout, nullptr );
		pipelineLayout = VK_NULL_HANDLE;
		vkDestroyPipeline( VulkanCommon::Device, pipeline, nullptr );
		//vkDestroyPipelineCache( VulkanCommon::Device, _pipelineCache, nullptr );
		pipeline = VK_NULL_HANDLE;
		//_pipelineCache = VK_NULL_HANDLE;
	}

	VkShaderModule Pipeline::CreateShaderModule( const char *filepath )
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
}