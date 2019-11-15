#include "vVulkanPipeline.h"

namespace Vel
{
	void Pipeline::Create( VkRenderPass renderPass, ShaderDescription shader )
	{
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo;
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		pipelineCacheCreateInfo.pNext = nullptr;
		pipelineCacheCreateInfo.flags = 0;
		pipelineCacheCreateInfo.initialDataSize = 0;
		pipelineCacheCreateInfo.pInitialData = nullptr;

		//vkCreatePipelineCache( VulkanCommon::Device, &pipelineCacheCreateInfo, nullptr, &pipelineCache ); //TODO add pipeline cache

		CreatePipeline( renderPass, shader );

		shader.DestroyShaderModules();
	}

	void Pipeline::CreatePipeline( VkRenderPass renderPass, ShaderDescription shader )
	{
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

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.flags = 0;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &shader.dscSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

		CheckResult( vkCreatePipelineLayout( VulkanCommon::Device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout ), "failed to create pipeline layout" );

		VkGraphicsPipelineCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.stageCount = 2;
		createInfo.pStages = shader.shaderStageInfo;
		createInfo.pVertexInputState = &shader.vertexInputStateInfo;
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
}