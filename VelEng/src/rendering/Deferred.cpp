#include "Deferred.h"
#include "PipelineBuilder.h"
#include "VulkanUtils.h"

#include <fstream>

void Vel::GPassPipeline::CreatePipeline(VkDescriptorSetLayout* layouts, uint32_t layoutsCount)
{
    VkShaderModule vertexModule;
    if (!LoadShaderModule(GET_SHADER_PATH("gpass.vert"), device, &vertexModule))
    {
        fmt::print("Error when building the vertex shader \n");
    }
    VkShaderModule fragmentModule;
    if (!LoadShaderModule(GET_SHADER_PATH("gpass.frag"), device, &fragmentModule))
    {
        fmt::print("Error when building the fragment shader \n");
    }

    VkPushConstantRange bufferRange{};
    bufferRange.offset = 0;
    bufferRange.size = sizeof(GPUDrawPushConstants);
    bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .setLayoutCount = layoutsCount,
        .pSetLayouts = layouts,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &bufferRange
    };

    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout));


    pipelineBuilder.Reset();
    pipelineBuilder.SetPipelineLayoutGPass(pipelineLayout, 3);
    pipelineBuilder.SetShaders(vertexModule, fragmentModule);
    pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
    pipelineBuilder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
    pipelineBuilder.SetMultisampling();
    pipelineBuilder.DisableBlending();

    VkFormat formats[] = { VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM };
    pipelineBuilder.SetColorAttachmentsFormats(formats, 3);
    pipelineBuilder.SetDepthFormat(VK_FORMAT_D32_SFLOAT);
    pipelineBuilder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);

    pipeline = pipelineBuilder.BuildGfxPipeline(device);

    vkDestroyShaderModule(device, vertexModule, nullptr);
    vkDestroyShaderModule(device, fragmentModule, nullptr);
}

void Vel::LPassPipeline::CreatePipeline(VkDescriptorSetLayout* layouts, uint32_t layoutsCount)
{
    VkShaderModule vertexModule;
    if (!LoadShaderModule(GET_SHADER_PATH("lpass.vert"), device, &vertexModule))
    {
        fmt::print("Error when building the vertex shader \n");
    }
    VkShaderModule fragmentModule;
    if (!LoadShaderModule(GET_SHADER_PATH("lpass.frag"), device, &fragmentModule))
    {
        fmt::print("Error when building the fragment shader \n");
    }

    VkPushConstantRange bufferRange{};
    bufferRange.offset = 0;
    bufferRange.size = sizeof(GPUDrawPushConstants);
    bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {
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
    //pipelineBuilder.SetDepthFormat(VK_FORMAT_D32_SFLOAT);
    pipelineBuilder.EnableDepthTest(false, VK_COMPARE_OP_GREATER_OR_EQUAL);

    pipeline = pipelineBuilder.BuildGfxPipeline(device);

    vkDestroyShaderModule(device, vertexModule, nullptr);
    vkDestroyShaderModule(device, fragmentModule, nullptr);
}

void Vel::DeferredRenderer::Init(VkDevice dev, GPUAllocator* allocator, VkExtent2D renderExtent,
    VkDescriptorSetLayout cameraDescriptorLayout,
    VkBuffer sceneLightDataBuffer, size_t sceneLightDataBufferSize,
    GPUMeshBuffers&& rect)
{
    drawExtent = renderExtent;
    device = dev;
    drawRect = std::move(rect);
    mainAllocator = allocator;

    VkSamplerCreateInfo samplerCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR
    };

    vkCreateSampler(device, &samplerCreateInfo, nullptr, &sampler);

    //TODO format doesnt matter now for now
    uint32_t color = glm::packUnorm4x8(glm::vec4(1.f, 1.f, 1.f, 1.f));
    defaultColorMap = mainAllocator->CreateImage((void*)&color, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, false);
    uint32_t normals = glm::packUnorm4x8(glm::vec4(0.5f, 0.5f, 1.f, 1.f));
    defaultNormalMap = mainAllocator->CreateImage((void*)&normals, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, false);
    uint32_t specular = glm::packUnorm4x8(glm::vec4(0.1f, 0.0f, 0.0f, 0.0f));
    defaultSpecularMap = mainAllocator->CreateImage((void*)&specular, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, false);

    VkImageUsageFlags gPassAttachmentsUsage { 
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT
    };

    VkExtent3D imageExtent{ drawExtent.width, drawExtent.height, 1 };
    framebuffer.position = mainAllocator->CreateImage(imageExtent, VK_FORMAT_R32G32B32A32_SFLOAT, gPassAttachmentsUsage, false);
    framebuffer.color = mainAllocator->CreateImage(imageExtent, VK_FORMAT_R8G8B8A8_UNORM, gPassAttachmentsUsage, false);
    framebuffer.normals = mainAllocator->CreateImage(imageExtent, VK_FORMAT_R8G8B8A8_UNORM, gPassAttachmentsUsage, false);
    framebuffer.depth = mainAllocator->CreateImage(imageExtent, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, false);

    DescriptorLayoutBuilder builder;
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    builder.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    builder.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    gPassDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

    std::vector<DescriptorPoolSizeRatio> sizes = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3.f },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3.f },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1.f }
    };
    descriptorPool.InitPool(device, 10, sizes);
    testGPassSet = descriptorPool.Allocate(gPassDescriptorLayout);

    descriptorWriter.Clear();
    descriptorWriter.WriteImage(0, defaultColorMap.imageView, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptorWriter.WriteImage(1, defaultNormalMap.imageView, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptorWriter.WriteImage(2, defaultSpecularMap.imageView, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptorWriter.UpdateSet(device, testGPassSet);

    VkDescriptorSetLayout gPassLayouts[] = { cameraDescriptorLayout, gPassDescriptorLayout };
    gPass.SetDevice(device);
    gPass.CreatePipeline(gPassLayouts, 2);

    VkImageUsageFlags lPassAttachmentsUsage{
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT
    };
    drawImage = mainAllocator->CreateImage(imageExtent, VK_FORMAT_R8G8B8A8_UNORM, lPassAttachmentsUsage, false);

    builder.Clear();
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    builder.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    builder.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    framebufferDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    framebufferDescriptorSet = descriptorPool.Allocate(framebufferDescriptorLayout);

    descriptorWriter.Clear();
    descriptorWriter.WriteImage(0, framebuffer.position.imageView, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptorWriter.WriteImage(1, framebuffer.color.imageView, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptorWriter.WriteImage(2, framebuffer.normals.imageView, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    descriptorWriter.UpdateSet(device, framebufferDescriptorSet);

    builder.Clear();
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    lightsDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

    lightsDescriptorSet = descriptorPool.Allocate(lightsDescriptorLayout);
    descriptorWriter.Clear();
    descriptorWriter.WriteBuffer(0, sceneLightDataBuffer, sceneLightDataBufferSize, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptorWriter.UpdateSet(device, lightsDescriptorSet);

    VkDescriptorSetLayout lPassLayouts[] = { cameraDescriptorLayout, framebufferDescriptorLayout, lightsDescriptorLayout };
    lPass.SetDevice(device);
    lPass.CreatePipeline(lPassLayouts, 3);

    VkSemaphoreCreateInfo semaphoreInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr
    };

    VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &gPassFinishDrawing));
    VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &lPassFinishDrawing));

    PreBuildRenderInfo();
}

void Vel::DeferredRenderer::PreBuildRenderInfo()
{
    framebufferAttachments[0] = BuildGPassAttachmentInfo(framebuffer.position.imageView);
    framebufferAttachments[1] = BuildGPassAttachmentInfo(framebuffer.color.imageView);
    framebufferAttachments[2] = BuildGPassAttachmentInfo(framebuffer.normals.imageView);
    gPassDepthAttachmentInfo = BuildDepthAttachmentInfo();
    gPassRenderInfo = BuildRenderInfo(framebufferAttachments, 3, &gPassDepthAttachmentInfo);

    lPassDrawAttachment = BuildLPassAttachmentInfo(drawImage.imageView);
    lPassRenderInfo = BuildRenderInfo(&lPassDrawAttachment, 1, nullptr);

    renderViewport = BuildRenderViewport();
    renderScissor = BuildRenderScissors();
}

VkRenderingAttachmentInfo Vel::DeferredRenderer::BuildGPassAttachmentInfo(VkImageView imageView)
{
    VkRenderingAttachmentInfo colorAttachment
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue =
        {
            .color = {0, 0, 0, 0}
        }
    };

    return colorAttachment;
}

VkRenderingAttachmentInfo Vel::DeferredRenderer::BuildLPassAttachmentInfo(VkImageView imageView)
{
    VkRenderingAttachmentInfo colorAttachment
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    };

    return colorAttachment;
}

VkRenderingAttachmentInfo Vel::DeferredRenderer::BuildDepthAttachmentInfo()
{
    VkRenderingAttachmentInfo depthAttachment
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = framebuffer.depth.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue
        {
            .depthStencil
            {
                .depth = 0.f
            }
        }
    };

    return depthAttachment;
}

VkRenderingInfo Vel::DeferredRenderer::BuildRenderInfo(VkRenderingAttachmentInfo* color, uint32_t colorAttachmentsCount, VkRenderingAttachmentInfo* depth)
{
    VkRenderingInfo renderInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext = nullptr,
        .renderArea = {.offset = { 0, 0 }, .extent = { drawExtent.width, drawExtent.height } },
        .layerCount = 1,
        .colorAttachmentCount = colorAttachmentsCount,
        .pColorAttachments = color,
        .pDepthAttachment = depth,
        .pStencilAttachment = nullptr
    };

    return renderInfo;
}

VkViewport Vel::DeferredRenderer::BuildRenderViewport()
{
    VkViewport viewport{
        .x = 0,
        .y = 0,
        .width = (float)drawExtent.width,
        .height = (float)drawExtent.height,
        .minDepth = 0.f,
        .maxDepth = 1.f
    };

    return viewport;
}

VkRect2D Vel::DeferredRenderer::BuildRenderScissors()
{
    VkRect2D scissorsRect{
        .offset = {
            .x = 0,
            .y = 0
        },
        .extent = {
            .width = drawExtent.width,
            .height = drawExtent.height
        }
    };
    return scissorsRect;
}

void Vel::DeferredRenderer::Cleanup()
{
    descriptorPool.Cleanup();
    mainAllocator->DestroyImage(defaultColorMap);
    mainAllocator->DestroyImage(defaultNormalMap);
    mainAllocator->DestroyImage(defaultSpecularMap);
    mainAllocator->DestroyImage(framebuffer.position);
    mainAllocator->DestroyImage(framebuffer.color);
    mainAllocator->DestroyImage(framebuffer.normals);
    mainAllocator->DestroyImage(framebuffer.depth);
    mainAllocator->DestroyImage(drawImage);
    mainAllocator->DestroyBuffer(drawRect.indexBuffer);
    mainAllocator->DestroyBuffer(drawRect.vertexBuffer);
    vkDestroySemaphore(device, gPassFinishDrawing, nullptr);
    vkDestroySemaphore(device, lPassFinishDrawing, nullptr);
    vkDestroyDescriptorSetLayout(device, gPassDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, framebufferDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, lightsDescriptorLayout, nullptr);
    gPass.Cleanup();
    lPass.Cleanup();
    vkDestroySampler(device, sampler, nullptr);
}

void Vel::DeferredRenderer::DrawGPass(const DrawContext& context, VkCommandBuffer cmd)
{
    TransitionImage(cmd, framebuffer.position.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    TransitionImage(cmd, framebuffer.color.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    TransitionImage(cmd, framebuffer.normals.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    TransitionImage(cmd, framebuffer.depth.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    vkCmdBeginRendering(cmd, &gPassRenderInfo);

    vkCmdSetViewport(cmd, 0, 1, &renderViewport);
    vkCmdSetScissor(cmd, 0, 1, &renderScissor);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gPass.GetPipeline());

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gPass.GetPipelineLayout(), 0, 1, &sceneCameraDataDescriptorSet, 0, nullptr);

    VkBuffer lastIndexBuffer = VK_NULL_HANDLE; //TODO
    for (const auto& materialDraws : context.opaqueSurfaces)
    {
        if (!materialDraws.empty())
        {
            auto material = materialDraws[0].materialData;
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gPass.GetPipelineLayout(), 1, 1, &testGPassSet, 0, nullptr);
            for (const auto& drawData : materialDraws)
            {
                lastIndexBuffer = drawData.indexBuffer;
                vkCmdBindIndexBuffer(cmd, drawData.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

                GPUDrawPushConstants pushConstants{
                    .worldMatrix = drawData.transform,
                    .vertexBuffer = drawData.vertexBufferAddress
                };

                vkCmdPushConstants(cmd, gPass.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);

                vkCmdDrawIndexed(cmd, drawData.indexCount, 1, drawData.firstIndex, 0, 0);
            }
        }
    }

    vkCmdEndRendering(cmd);
}

void Vel::DeferredRenderer::DrawLPass(const DrawContext& context, VkCommandBuffer cmd)
{
    TransitionImage(cmd, framebuffer.position.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    TransitionImage(cmd, framebuffer.color.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    TransitionImage(cmd, framebuffer.normals.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    TransitionImage(cmd, drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    vkCmdBeginRendering(cmd, &lPassRenderInfo);

    vkCmdSetViewport(cmd, 0, 1, &renderViewport);
    vkCmdSetScissor(cmd, 0, 1, &renderScissor);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, lPass.GetPipeline());

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, lPass.GetPipelineLayout(), 0, 1, &sceneCameraDataDescriptorSet, 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, lPass.GetPipelineLayout(), 1, 1, &framebufferDescriptorSet, 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, lPass.GetPipelineLayout(), 2, 1, &lightsDescriptorSet, 0, nullptr);

    GPUDrawPushConstants pushConstants;
    pushConstants.worldMatrix = glm::mat4{ 1.f };
    pushConstants.vertexBuffer = drawRect.vertexBufferAddress;

    vkCmdPushConstants(cmd, lPass.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);
    vkCmdBindIndexBuffer(cmd, drawRect.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);

    vkCmdEndRendering(cmd);
}
