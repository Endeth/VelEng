#include "Rendering/RenderPasses/SkyboxPass.h"

void Vel::SkyboxPipeline::CreatePipeline(VkDescriptorSetLayout* layouts, uint32_t layoutsCount)
{
    VkShaderModule vertexModule;
    if (!LoadShaderModule(GET_SHADER_PATH("skybox.vert"), device, &vertexModule))
    {
        fmt::print("Error when building the vertex shader \n");
    }
    VkShaderModule fragmentModule;
    if (!LoadShaderModule(GET_SHADER_PATH("skybox.frag"), device, &fragmentModule))
    {
        fmt::print("Error when building the fragment shader \n");
    }

    VkPushConstantRange bufferRange{
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(SkyboxPushConstants)
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .setLayoutCount = layoutsCount,
        .pSetLayouts = layouts,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &bufferRange
    };

    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

    pipelineBuilder.Reset();
    pipelineBuilder.SetPipelineLayout(pipelineLayout);
    pipelineBuilder.SetShaders(vertexModule, fragmentModule);
    pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
    pipelineBuilder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
    pipelineBuilder.SetMultisampling();
    pipelineBuilder.DisableBlending();

    pipelineBuilder.SetColorAttachmentFormat(VK_FORMAT_R8G8B8A8_UNORM);
    pipelineBuilder.SetDepthFormat(VK_FORMAT_D32_SFLOAT);
    pipelineBuilder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);

    pipeline = pipelineBuilder.BuildGfxPipeline(device);

    vkDestroyShaderModule(device, vertexModule, nullptr);
    vkDestroyShaderModule(device, fragmentModule, nullptr);
}

void Vel::SkyboxPass::Init(VkDevice dev, AllocatableImage& skyboxImage)
{
    device = dev;

    VkSamplerCreateInfo samplerCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR
    };

    vkCreateSampler(device, &samplerCreateInfo, nullptr, &sampler);

    DescriptorLayoutBuilder builder;
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    skyboxLayout = builder.Build(device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

    std::vector<DescriptorPoolSizeRatio> sizes = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3.f },
    };
    descriptorPool.InitPool(device, 10, sizes);

    pipeline.SetDevice(device);
    pipeline.CreatePipeline(&skyboxLayout, 1);

    skyboxDescriptorSet = descriptorPool.Allocate(skyboxLayout);
    descriptorWriter.Clear();
    descriptorWriter.WriteImageSampler(0, skyboxImage.imageView, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    descriptorWriter.UpdateSet(device, skyboxDescriptorSet);

    PrebuildRenderInfo();
}

void Vel::SkyboxPass::Draw(VkCommandBuffer cmd, const Camera& camera, const AllocatableImage& drawImage)
{
    VkExtent2D drawExtent = { .width = drawImage.extent.width, .height = drawImage.extent.height };
    renderingAttachmentInfo.imageView = drawImage.imageView;

    renderingInfo.renderArea = {
        .offset = { 0, 0 },
        .extent = drawExtent
    };

    renderViewport.width = (float)drawExtent.width;
    renderViewport.height = (float)drawExtent.height;

    renderScissor.extent = drawExtent;

    vkCmdBeginRendering(cmd, &renderingInfo);
    vkCmdSetViewport(cmd, 0, 1, &renderViewport);

    vkCmdSetScissor(cmd, 0, 1, &renderScissor);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipeline());

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipelineLayout(), 0, 1, &skyboxDescriptorSet, 0, nullptr);
    SkyboxPushConstants pushConstants;
    pushConstants.inverseViewProjection = glm::inverse(camera.GetViewProjectionMatrix());
    pushConstants.cameraPosition = glm::vec4(camera.GetPosition(), 1.0f);

    vkCmdPushConstants(cmd, pipeline.GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SkyboxPushConstants), &pushConstants);
    vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdEndRendering(cmd);
}

void Vel::SkyboxPass::Cleanup()
{
    descriptorPool.Cleanup();
    vkDestroyDescriptorSetLayout(device, skyboxLayout, nullptr);
    pipeline.Cleanup();
    vkDestroySampler(device, sampler, nullptr);
}

void Vel::SkyboxPass::PrebuildRenderInfo()
{
    renderingAttachmentInfo = BuildAttachmentInfo();
    renderingInfo = BuildRenderInfo();

    renderViewport = BuildRenderViewport();
    renderScissor = BuildRenderScissors();
}

VkRenderingAttachmentInfo Vel::SkyboxPass::BuildAttachmentInfo()
{
    VkRenderingAttachmentInfo colorAttachment
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = drawImageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    };

    return colorAttachment;
}

VkRenderingInfo Vel::SkyboxPass::BuildRenderInfo()
{
    VkRenderingInfo renderInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext = nullptr,
        //.renderArea = {.offset = { 0, 0 }, .extent = { drawExtent.width, drawExtent.height } },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &renderingAttachmentInfo,
        .pDepthAttachment = nullptr,
        .pStencilAttachment = nullptr
    };

    return renderInfo;
}

VkViewport Vel::SkyboxPass::BuildRenderViewport()
{
    VkViewport viewport{
        .x = 0,
        .y = 0,
        .minDepth = 0.f,
        .maxDepth = 1.f
    };

    return viewport;
}

VkRect2D Vel::SkyboxPass::BuildRenderScissors()
{
    VkRect2D scissorsRect{
        .offset = {
            .x = 0,
            .y = 0
        }
    };
    return scissorsRect;
}
