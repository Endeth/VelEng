#include "rendering/PipelineBuilder.h"

void Vel::VkPipelineBuilder::Init(VkDevice dev)
{
    device = dev;
    Reset();
}

void Vel::VkPipelineBuilder::Reset()
{
    pipelineLayout = VK_NULL_HANDLE;
    shaderStages.clear();
    inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    colorBlendAttachment[0] = {};
    multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    depthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    renderInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
}

void Vel::VkPipelineBuilder::SetPipelineLayout(VkPipelineLayout layout)
{
    pipelineLayout = layout;
    colorAttachmentsCount = 1;
}

void Vel::VkPipelineBuilder::SetPipelineLayoutGPass(VkPipelineLayout layout, uint32_t colAttachmentsCount)
{
    pipelineLayout = layout;
    colorAttachmentsCount = colAttachmentsCount;
}

void Vel::VkPipelineBuilder::SetShaders(VkShaderModule vertex, VkShaderModule fragment)
{
    shaderStages.clear();

    VkPipelineShaderStageCreateInfo vertStageInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertex,
        .pName = "main"
    };

    VkPipelineShaderStageCreateInfo fragStageInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragment,
        .pName = "main"
    };

    shaderStages.push_back(vertStageInfo);
    shaderStages.push_back(fragStageInfo);
}

void Vel::VkPipelineBuilder::SetInputTopology(VkPrimitiveTopology topology)
{
    inputAssembly.topology = topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
}

void Vel::VkPipelineBuilder::SetPolygonMode(VkPolygonMode mode)
{
    rasterizer.polygonMode = mode;
    rasterizer.lineWidth = 1.f;
}

void Vel::VkPipelineBuilder::SetCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace)
{
    rasterizer.cullMode = cullMode;
    rasterizer.frontFace = frontFace;
}

void Vel::VkPipelineBuilder::SetMultisampling()
{
    multisampling.sampleShadingEnable = VK_FALSE;
    // multisampling defaulted to no multisampling (1 sample per pixel)
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    // no alpha to coverage either
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;
}

void Vel::VkPipelineBuilder::EnableBlendingAdditive()
{
    colorBlendAttachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment[0].blendEnable = VK_TRUE;
    colorBlendAttachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment[0].colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment[0].alphaBlendOp = VK_BLEND_OP_ADD;
}

void Vel::VkPipelineBuilder::EnableBlendingAlphablend()
{
    colorBlendAttachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment[0].blendEnable = VK_TRUE;
    colorBlendAttachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment[0].colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment[0].alphaBlendOp = VK_BLEND_OP_ADD;
}

void Vel::VkPipelineBuilder::DisableBlending()
{
    colorBlendAttachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment[0].blendEnable = VK_FALSE;
}

void Vel::VkPipelineBuilder::SetColorAttachmentFormat(VkFormat format)
{
    colorAttachmentFormat = format;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachmentFormats = &colorAttachmentFormat;
}

void Vel::VkPipelineBuilder::SetColorAttachmentsFormats(VkFormat* formats, uint32_t formatsCount)
{
    renderInfo.colorAttachmentCount = formatsCount;
    renderInfo.pColorAttachmentFormats = formats;
}

void Vel::VkPipelineBuilder::SetDepthFormat(VkFormat format)
{
    renderInfo.depthAttachmentFormat = format;
}

void Vel::VkPipelineBuilder::EnableDepthTest(bool depthWriteEnable, VkCompareOp op)
{
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = depthWriteEnable;
    depthStencil.depthCompareOp = op;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};
    depthStencil.minDepthBounds = 0.f;
    depthStencil.maxDepthBounds = 1.f;
}

void Vel::VkPipelineBuilder::DisableDepthTest()
{
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};
    depthStencil.minDepthBounds = 0.f;
    depthStencil.maxDepthBounds = 1.f;
}

VkPipeline Vel::VkPipelineBuilder::BuildGfxPipeline(VkDevice device)
{
    VkPipeline pipeline;

    //Constant
    VkPipelineViewportStateCreateInfo viewportState {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    VkPipelineColorBlendStateCreateInfo colorBlending {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr
    };

    for (uint32_t i = 1; i < colorAttachmentsCount; ++i)
    {
        memcpy(colorBlendAttachment + i, colorBlendAttachment, sizeof(VkPipelineColorBlendAttachmentState));
    }

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = colorAttachmentsCount;
    colorBlending.pAttachments = colorBlendAttachment;

    VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicInfo { 
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = &state[0]
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };


    VkGraphicsPipelineCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderInfo,

        .stageCount = (uint32_t)shaderStages.size(),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicInfo,
        .layout = pipelineLayout
    };

    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline));
    return pipeline;
}
