#include "Rendering/RenderPasses/ShadowPass.h"

#include "Rendering/VulkanUtils.h"

void Vel::ShadowPipeline::CreatePipeline(VkDescriptorSetLayout* layouts, uint32_t layoutsCount)
{
    VkShaderModule vertexModule;
    if (!LoadShaderModule(GET_SHADER_PATH("shadow_directional_light.vert"), device, &vertexModule))
    {
        fmt::print("Error when building the vertex shader \n");
    }
    VkShaderModule fragmentModule;
    if (!LoadShaderModule(GET_SHADER_PATH("shadow_directional_light.frag"), device, &fragmentModule))
    {
        fmt::print("Error when building the fragment shader \n");
    }

    VkPushConstantRange bufferRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(GPUDrawPushConstants)
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
    pipelineBuilder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);
    pipelineBuilder.SetMultisampling();
    pipelineBuilder.DisableBlending();

    //pipelineBuilder.SetColorAttachmentFormat(VK_FORMAT_R8G8B8A8_UNORM);
    pipelineBuilder.SetDepthFormat(VK_FORMAT_D32_SFLOAT);
    pipelineBuilder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);

    pipeline = pipelineBuilder.BuildGfxPipeline(device);

    vkDestroyShaderModule(device, vertexModule, nullptr);
    vkDestroyShaderModule(device, fragmentModule, nullptr);
}

void Vel::ShadowPass::Init(VkDevice dev, Sunlight& sunlight)
{
    device = dev;
    drawExtent = sunlight.shadowResolution;
    drawImageView = sunlight.shadowMap.imageView;
    drawImage = sunlight.shadowMap.image;

    DescriptorLayoutBuilder builder;
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    shadowPassLayout = builder.Build(device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

    std::vector<DescriptorPoolSizeRatio> sizes = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3.f },
    };
    descriptorPool.InitPool(device, 10, sizes);

    pipeline.SetDevice(device);
    pipeline.CreatePipeline(&shadowPassLayout, 1);

    shadowPassDescriptorSet = descriptorPool.Allocate(shadowPassLayout);
    descriptorWriter.Clear();
    descriptorWriter.WriteBuffer(0, sunlight.gpuViewProjData.buffer, sizeof(glm::mat4), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    descriptorWriter.UpdateSet(device, shadowPassDescriptorSet);

    PrebuildRenderInfo();
}

void Vel::ShadowPass::Draw(const DrawContext& context, VkCommandBuffer cmd)
{
    TransitionDepthImage(cmd, drawImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    vkCmdBeginRendering(cmd, &renderingInfo);

    vkCmdSetViewport(cmd, 0, 1, &renderViewport);
    vkCmdSetScissor(cmd, 0, 1, &renderScissor);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipeline());

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipelineLayout(), 0, 1, &shadowPassDescriptorSet, 0, nullptr);

    VkBuffer lastIndexBuffer = VK_NULL_HANDLE;
    for (const auto& materialDraws : context.opaqueSurfaces)
    {
        for (const auto& drawData : materialDraws)
        {
            if (lastIndexBuffer != drawData.indexBuffer)
            {
                lastIndexBuffer = drawData.indexBuffer;
                vkCmdBindIndexBuffer(cmd, drawData.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            }

            GPUDrawPushConstants pushConstants{
                .worldMatrix = drawData.transform,
                .vertexBuffer = drawData.vertexBufferAddress
            };

            vkCmdPushConstants(cmd, pipeline.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);

            vkCmdDrawIndexed(cmd, drawData.indexCount, 1, drawData.firstIndex, 0, 0);
        }
    }

    vkCmdEndRendering(cmd);
}

void Vel::ShadowPass::Cleanup()
{
    descriptorPool.Cleanup();
    vkDestroyDescriptorSetLayout(device, shadowPassLayout, nullptr);
    pipeline.Cleanup();
}

void Vel::ShadowPass::PrebuildRenderInfo()
{
    depthAttachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = drawImageView,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = { .depthStencil = 0.0f }
    };

    renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext = nullptr,
        .renderArea = {.offset = { 0, 0 }, .extent = { drawExtent.width, drawExtent.height } },
        .layerCount = 1,
        .colorAttachmentCount = 0,
        .pColorAttachments = nullptr,
        .pDepthAttachment = &depthAttachment,
        .pStencilAttachment = nullptr
    };

    renderViewport = {
        .x = 0,
        .y = 0,
        .width = (float)drawExtent.width,
        .height = (float)drawExtent.height,
        .minDepth = 0.f,
        .maxDepth = 1.f
    };

    renderScissor = {
        .offset = {
            .x = 0,
            .y = 0
        },
        .extent = {
            .width = drawExtent.width,
            .height = drawExtent.height
        }
    };
}
