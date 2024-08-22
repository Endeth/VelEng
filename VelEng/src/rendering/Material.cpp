#include "Material.h"
#include "PipelineBuilder.h"

void Vel::GLTFMetallicRoughness::Init(VkDevice dev, VkDescriptorSetLayout sceneDescriptorsLayout, VkFormat drawImgFormat, VkFormat depthImgFormat)
{
    device = dev;
    sceneLayout = sceneDescriptorsLayout;
    drawImageFormat = drawImgFormat;
    depthImageFormat = depthImgFormat;
}

void Vel::GLTFMetallicRoughness::BuildPipelines()
{
    VkShaderModule vertexModule;
    if (!LoadShaderModule(GET_SHADER_PATH("gltf_mesh.vert"), device, &vertexModule))
    {
        fmt::print("Error when building the vertex shader \n");
    }
    VkShaderModule fragmentModule;
    if (!LoadShaderModule(GET_SHADER_PATH("gltf_mesh.frag"), device, &fragmentModule))
    {
        fmt::print("Error when building the fragment shader \n");
    }

    VkPushConstantRange matrixRange {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(GPUDrawPushConstants)
    };

    DescriptorLayoutBuilder descLayoutBuilder;
    descLayoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descLayoutBuilder.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descLayoutBuilder.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descLayoutBuilder.AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    materialLayout = descLayoutBuilder.Build(device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayout layouts[] = { sceneLayout, materialLayout };

    VkPipelineLayoutCreateInfo meshLayoutInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .setLayoutCount = 2,
        .pSetLayouts = layouts,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &matrixRange
    };

    VkPipelineLayout newLayout;
    VK_CHECK(vkCreatePipelineLayout(device, &meshLayoutInfo, nullptr, &newLayout));

    opaquePipeline.pipelineLayout = newLayout;
    transparentPipeline.pipelineLayout = newLayout;

    VkPipelineBuilder pipelineBuilder;
    pipelineBuilder.Init(device);
    pipelineBuilder.SetPipelineLayout(newLayout);
    pipelineBuilder.SetShaders(vertexModule, fragmentModule);
    pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
    pipelineBuilder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
    pipelineBuilder.SetMultisampling();
    pipelineBuilder.SetColorAttachmentFormat(drawImageFormat);
    pipelineBuilder.SetDepthFormat(depthImageFormat);

    pipelineBuilder.DisableBlending();
    pipelineBuilder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
    opaquePipeline.pipeline = pipelineBuilder.BuildGfxPipeline(device);

    pipelineBuilder.EnableBlendingAdditive();
    pipelineBuilder.EnableDepthTest(false, VK_COMPARE_OP_GREATER_OR_EQUAL);
    transparentPipeline.pipeline = pipelineBuilder.BuildGfxPipeline(device);

    vkDestroyShaderModule(device, vertexModule, nullptr);
    vkDestroyShaderModule(device, fragmentModule, nullptr);
}

void Vel::GLTFMetallicRoughness::ClearResources()
{
    vkDestroyPipelineLayout(device, opaquePipeline.pipelineLayout, nullptr); //Shared with transparent
    vkDestroyPipeline(device, opaquePipeline.pipeline, nullptr);
    vkDestroyPipeline(device, transparentPipeline.pipeline, nullptr);
    vkDestroyDescriptorSetLayout(device, materialLayout, nullptr);
}

Vel::MaterialInstance Vel::GLTFMetallicRoughness::WriteMaterial(MaterialPass pass, const MaterialResources& resources, DescriptorAllocatorDynamic& descriptorAllocator)
{
    MaterialInstance materialInstance;
    materialInstance.passType = pass;
    //materialInstance.pipeline = pass == MaterialPass::MainColor ? &opaquePipeline : &transparentPipeline;
    materialInstance.descriptorSet = descriptorAllocator.Allocate(materialLayout);

    writer.Clear();
    writer.WriteBuffer(0, resources.dataBuffer, sizeof(MaterialConstants), resources.dataBufferOffset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    writer.WriteImageSampler(1, resources.colorImage.imageView, resources.colorSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    writer.WriteImageSampler(2, resources.normalsImage.imageView, resources.normalsSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    writer.WriteImageSampler(3, resources.metallicRoughnessImage.imageView, resources.metallicRoughnessSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    writer.UpdateSet(device, materialInstance.descriptorSet);

    return materialInstance;
}
