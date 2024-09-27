#include "Rendering/RenderPasses/DeferredPasses.h"

#include <fstream>

#include "Rendering/VulkanUtils.h"

#include "Rendering/RenderPasses/PipelineBuilder.h"


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


    VkFormat formats[] = { VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_UNORM };
    constexpr size_t attachmentsCount = sizeof(formats) / sizeof(VkFormat);

    pipelineBuilder.Reset();
    pipelineBuilder.SetPipelineLayoutGPass(pipelineLayout, attachmentsCount);
    pipelineBuilder.SetShaders(vertexModule, fragmentModule);
    pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
    pipelineBuilder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
    pipelineBuilder.SetMultisampling();
    pipelineBuilder.DisableBlending();

    pipelineBuilder.SetColorAttachmentsFormats(formats, attachmentsCount);
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

    //TODO can be removed after vertex magic

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .setLayoutCount = layoutsCount,
        .pSetLayouts = layouts,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
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

void Vel::DeferredPasses::Init(VkDevice dev, VkDescriptorSetLayout cameraDescriptorLayout)
{
    device = dev;

    CreateSamplers();

    //TODO sizes are based on gPass only but are used in lPass
    std::vector<DescriptorPoolSizeRatio> sizes = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3.f },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3.f },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1.f }
    };
    descriptorPool.InitPool(device, 10, sizes);

    CreateGPassDescriptorLayouts();

    VkDescriptorSetLayout gPassLayouts[] = { cameraDescriptorLayout, gPassDescriptorLayout };
    constexpr uint32_t gPassLayoutsCount = sizeof(gPassLayouts) / sizeof(VkDescriptorSetLayout);
    gPass.SetDevice(device);
    gPass.CreatePipeline(gPassLayouts, gPassLayoutsCount);

    CreateLPassDescriptorLayouts();

    //THREAD UNSAFE
    //lightsDescriptorSet = descriptorPool.Allocate(lightsDescriptorLayout);

    VkDescriptorSetLayout lPassLayouts[] = { cameraDescriptorLayout, framebufferDescriptorLayout, lightsDescriptorLayout };
    constexpr uint32_t lPassLayoutsCount = sizeof(lPassLayouts) / sizeof(VkDescriptorSetLayout);
    lPass.SetDevice(device);
    lPass.CreatePipeline(lPassLayouts, lPassLayoutsCount);

    BuildRenderInfo();
}

void Vel::DeferredPasses::CreateSamplers()
{
    VkSamplerCreateInfo samplerCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR
    };
    vkCreateSampler(device, &samplerCreateInfo, nullptr, &sampler);

    VkSamplerCreateInfo shadowsSamplerCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST
    };
    vkCreateSampler(device, &shadowsSamplerCreateInfo, nullptr, &shadowsSampler);
}

void Vel::DeferredPasses::CreateGPassDescriptorLayouts()
{
    DescriptorLayoutBuilder builder;

    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); //Material color
    builder.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); //Material Normals
    builder.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); //Material MetallicRoughness
    gPassDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
}

void Vel::DeferredPasses::CreateLPassDescriptorLayouts()
{
    DescriptorLayoutBuilder builder;

    builder.Clear();
    builder.AddBinding(Framebuffer::POSITION, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    builder.AddBinding(Framebuffer::COLOR, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    builder.AddBinding(Framebuffer::NORMALS, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    builder.AddBinding(Framebuffer::METALLIC_ROUGHNESS, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    framebufferDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

    builder.Clear();
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); //Scene light data buffer
    builder.AddBinding(1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE); //Sunlight shadow map
    builder.AddBinding(2, VK_DESCRIPTOR_TYPE_SAMPLER);
    lightsDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
}

void Vel::DeferredPasses::BuildRenderInfo()
{
    framebufferAttachments[Framebuffer::POSITION] = BuildAttachmentInfo();
    framebufferAttachments[Framebuffer::COLOR] = BuildAttachmentInfo(VK_ATTACHMENT_LOAD_OP_LOAD); //Has Skybox data
    framebufferAttachments[Framebuffer::NORMALS] = BuildAttachmentInfo();
    framebufferAttachments[Framebuffer::METALLIC_ROUGHNESS] = BuildAttachmentInfo();
    gPassDepthAttachmentInfo = BuildDepthAttachmentInfo();
    gPassRenderInfo = BuildRenderInfo(framebufferAttachments, 4, &gPassDepthAttachmentInfo);

    lPassDrawAttachment = BuildAttachmentInfo(VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    lPassRenderInfo = BuildRenderInfo(&lPassDrawAttachment, 1, nullptr);

    renderViewport = BuildRenderViewport();
    renderScissor = BuildRenderScissors();
}

VkRenderingAttachmentInfo Vel::DeferredPasses::BuildAttachmentInfo(VkAttachmentLoadOp loadOp)
{
    VkRenderingAttachmentInfo colorAttachment
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = loadOp,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue =
        {
            .color = {0.0f, 0.0f, 0.0f, 0.0f}
        }
    };

    return colorAttachment;
}

VkRenderingAttachmentInfo Vel::DeferredPasses::BuildDepthAttachmentInfo()
{
    VkRenderingAttachmentInfo depthAttachment
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue
        {
            .depthStencil = {0.0f, 0}
        }
    };

    return depthAttachment;
}

VkRenderingInfo Vel::DeferredPasses::BuildRenderInfo(VkRenderingAttachmentInfo* color, uint32_t colorAttachmentsCount, VkRenderingAttachmentInfo* depth)
{
    VkRenderingInfo renderInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext = nullptr,
        .layerCount = 1,
        .colorAttachmentCount = colorAttachmentsCount,
        .pColorAttachments = color,
        .pDepthAttachment = depth,
        .pStencilAttachment = nullptr
    };

    return renderInfo;
}

VkViewport Vel::DeferredPasses::BuildRenderViewport()
{
    VkViewport viewport{
        .x = 0,
        .y = 0,
        .minDepth = 0.f,
        .maxDepth = 1.f
    };

    return viewport;
}

VkRect2D Vel::DeferredPasses::BuildRenderScissors()
{
    VkRect2D scissorsRect{
        .offset = {
            .x = 0,
            .y = 0
        },
    };
    return scissorsRect;
}

void Vel::DeferredPasses::Cleanup()
{
    descriptorPool.Cleanup();
    vkDestroyDescriptorSetLayout(device, gPassDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, framebufferDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, lightsDescriptorLayout, nullptr);
    gPass.Cleanup();
    lPass.Cleanup();
    vkDestroySampler(device, sampler, nullptr);
    vkDestroySampler(device, shadowsSampler, nullptr);
}

Vel::MaterialInstance Vel::DeferredPasses::CreateMaterialInstance(const DeferredMaterialResources& resources, DescriptorAllocatorDynamic& descriptorAllocator) const
{
    MaterialInstance materialInstance;
    materialInstance.descriptorSet = descriptorAllocator.Allocate(gPassDescriptorLayout);
 
    DescriptorWriter descriptorWriter;
    descriptorWriter.Clear();

    descriptorWriter.WriteImageSampler(0, resources.colorImage.imageView, resources.colorSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptorWriter.WriteImageSampler(1, resources.normalsImage.imageView, resources.normalsSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptorWriter.WriteImageSampler(2, resources.metallicRoughnessImage.imageView, resources.metallicRoughnessSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    descriptorWriter.UpdateSet(device, materialInstance.descriptorSet);

    return materialInstance;
}

Vel::DeferredPasses::Framebuffer Vel::DeferredPasses::CreateUnallocatedFramebuffer(const VkExtent3D& extent)
{
    Framebuffer framebuffer;

    VkImageUsageFlags gPassAttachmentsUsage {
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT |
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT //For debug show
    };

    //VkExtent3D imageExtent{ drawExtent.width, drawExtent.height, 1 }; //TODO: RESIZE;
    framebuffer.position.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    framebuffer.position.extent = extent;
    framebuffer.position.usageFlags = gPassAttachmentsUsage;
    framebuffer.color.format = VK_FORMAT_R8G8B8A8_UNORM;
    framebuffer.color.extent = extent;
    framebuffer.color.usageFlags = gPassAttachmentsUsage;
    framebuffer.normals.format = VK_FORMAT_R8G8B8A8_SNORM;
    framebuffer.normals.extent = extent;
    framebuffer.normals.usageFlags = gPassAttachmentsUsage;
    framebuffer.metallicRoughness.format = VK_FORMAT_R8G8B8A8_UNORM;
    framebuffer.metallicRoughness.extent = extent;
    framebuffer.metallicRoughness.usageFlags = gPassAttachmentsUsage;

    //TODO why do i need depth since I have position?
    framebuffer.depth.format = VK_FORMAT_D32_SFLOAT;
    framebuffer.depth.extent = extent;
    framebuffer.depth.usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    framebuffer.framebufferDescriptor = descriptorPool.Allocate(framebufferDescriptorLayout);

    return framebuffer;
}

Vel::AllocatableImage Vel::DeferredPasses::CreateUnallocatedLPassDrawImage(const VkExtent3D& extent)
{
    AllocatableImage image;
    image.format = VK_FORMAT_R8G8B8A8_UNORM;
    image.extent = extent;
    image.usageFlags = { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT };
    return image;
}

void Vel::DeferredPasses::SetRenderExtent(const VkExtent2D& extent)
{
    gPassRenderInfo.renderArea = {
        .offset = { 0, 0 },
        .extent = extent
    };

    lPassRenderInfo.renderArea = {
        .offset = { 0, 0 },
        .extent = extent
    };

    renderViewport.width = (float)extent.width;
    renderViewport.height = (float)extent.height;

    renderScissor.extent = extent;
}

void Vel::DeferredPasses::SetFramebufferGPassAttachment(const Framebuffer& framebuffer)
{
    //TODO TEMPLATE; make a templated function for framebuffer to get proper image by enum and compile loop
    framebufferAttachments[Framebuffer::POSITION].imageView = framebuffer.position.imageView;
    framebufferAttachments[Framebuffer::COLOR].imageView = framebuffer.color.imageView;
    framebufferAttachments[Framebuffer::NORMALS].imageView = framebuffer.normals.imageView;
    framebufferAttachments[Framebuffer::METALLIC_ROUGHNESS].imageView = framebuffer.metallicRoughness.imageView;

    gPassDepthAttachmentInfo.imageView = framebuffer.depth.imageView;
}

void Vel::DeferredPasses::UpdateFramebufferDescriptor(Framebuffer& framebuffer) const
{
    DescriptorWriter descriptorWriter;

    VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkDescriptorType descType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    //TODO TEMPLATE; make a templated function for framebuffer to get proper image by enum and compile loop
    descriptorWriter.WriteImageSampler(Framebuffer::POSITION, framebuffer.position.imageView, sampler, layout, descType);
    descriptorWriter.WriteImageSampler(Framebuffer::COLOR, framebuffer.color.imageView, sampler, layout, descType);
    descriptorWriter.WriteImageSampler(Framebuffer::NORMALS, framebuffer.normals.imageView, sampler, layout, descType);
    descriptorWriter.WriteImageSampler(Framebuffer::METALLIC_ROUGHNESS, framebuffer.metallicRoughness.imageView, sampler, layout, descType);

    descriptorWriter.UpdateSet(device, framebuffer.framebufferDescriptor);
}

void Vel::DeferredPasses::DrawGPass(const std::vector<DrawContext>& contexts, VkCommandBuffer cmd, const Framebuffer& framebuffer, const VkDescriptorSet& scene)
{
    SetFramebufferGPassAttachment(framebuffer);

    vkCmdBeginRendering(cmd, &gPassRenderInfo);

    vkCmdSetViewport(cmd, 0, 1, &renderViewport);
    vkCmdSetScissor(cmd, 0, 1, &renderScissor);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gPass.GetPipeline());

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gPass.GetPipelineLayout(), 0, 1, &scene, 0, nullptr);

    VkBuffer lastIndexBuffer = VK_NULL_HANDLE;
    for (const auto& context : contexts)
    {
        for (const auto& materialDraws : context.opaqueSurfaces)
        {
            if (!materialDraws.empty())
            {
                auto material = materialDraws[0].materialData;
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gPass.GetPipelineLayout(), 1, 1, &material->descriptorSet, 0, nullptr);
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

                    vkCmdPushConstants(cmd, gPass.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);

                    vkCmdDrawIndexed(cmd, drawData.indexCount, 1, drawData.firstIndex, 0, 0);
                }
            }
        }
    }

    vkCmdEndRendering(cmd);
}

void Vel::DeferredPasses::DrawLPass(VkCommandBuffer cmd, const AllocatableImage& drawImage, const VkDescriptorSet& scene, const VkDescriptorSet& frame, const VkDescriptorSet& light)
{
    lPassDrawAttachment.imageView = drawImage.imageView;

    vkCmdBeginRendering(cmd, &lPassRenderInfo);

    vkCmdSetViewport(cmd, 0, 1, &renderViewport);
    vkCmdSetScissor(cmd, 0, 1, &renderScissor);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, lPass.GetPipeline());

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, lPass.GetPipelineLayout(), 0, 1, &scene, 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, lPass.GetPipelineLayout(), 1, 1, &frame, 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, lPass.GetPipelineLayout(), 2, 1, &light, 0, nullptr);

    vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdEndRendering(cmd);
}

Vel::DeferredPasses::Framebuffer::Dependencies Vel::DeferredPasses::Framebuffer::GetPipelineBarrierDependencyInfo(const ImageBarrierInfo& src, const ImageBarrierInfo& dst)
{
    VkImageMemoryBarrier2 barrier = GetImageMemoryBarrier(src.stageMask, src.accessFlags, src.layout, dst.stageMask, dst.accessFlags, dst.layout);

    Dependencies imgBarriers;
    imgBarriers[POSITION] = barrier;
    imgBarriers[POSITION].image = position.image;
    imgBarriers[COLOR] = barrier;
    imgBarriers[COLOR].image = color.image;
    imgBarriers[NORMALS] = barrier;
    imgBarriers[NORMALS].image = normals.image;
    imgBarriers[METALLIC_ROUGHNESS] = barrier;
    imgBarriers[METALLIC_ROUGHNESS].image = metallicRoughness.image;

    return imgBarriers;
}

void Vel::DeferredPasses::Framebuffer::TransitionImages(VkCommandBuffer cmd, VkImageLayout src, VkImageLayout dst)
{
    TransitionImage(cmd, position.image, src, dst);
    TransitionImage(cmd, color.image, src, dst);
    TransitionImage(cmd, normals.image, src, dst);
    TransitionImage(cmd, metallicRoughness.image, src, dst);
}

void Vel::DeferredPasses::Framebuffer::TransitionImagesForAttachment(VkCommandBuffer cmd)
{
    TransitionImages(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    //TODO temp
    TransitionDepthImage(cmd, depth.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
}

void Vel::DeferredPasses::Framebuffer::TransitionImagesForDescriptors(VkCommandBuffer cmd)
{
    TransitionImages(cmd, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
