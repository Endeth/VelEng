#include "Rendering/Scene/Renderable.h"

void Vel::RenderableNode::SetLocalTransform(const glm::mat4& matrix)
{
    localTransform = matrix;
}

void Vel::RenderableNode::SetWorldTransform(const glm::mat4& matrix)
{
    worldTransform = matrix;
}

void Vel::RenderableNode::RefreshTransform(const glm::mat4& parentMatrix)
{
    worldTransform = parentMatrix * localTransform;
    for (auto& child : children)
    {
        child->RefreshTransform(worldTransform);
    }
}

void Vel::RenderableNode::Draw(const glm::mat4& topMatrix, DrawContext& context)
{
    for (auto& child : children)
    {
        child->Draw(topMatrix, context);
    }
}

Vel::MeshNode::MeshNode(const std::shared_ptr<MeshAsset>& m)
{
    mesh = m;
}

void Vel::MeshNode::Draw(const glm::mat4& topMatrix, DrawContext& context)
{
    glm::mat4 nodeMatrix = topMatrix * worldTransform;

    RenderData data;
    for (auto& surface : mesh->surfaces)
    {
        data.indexCount = surface.count;
        data.firstIndex = surface.startIndex;
        data.indexBuffer = mesh->meshBuffers.indexBuffer.buffer;
        data.materialData = surface.materialInstance.get();

        data.transform = nodeMatrix;
        data.vertexBufferAddress = mesh->meshBuffers.vertexBufferAddress;

        if (surface.materialInstance->passType == MaterialPass::Transparent)
        {
            //context.transparentSurfaces[surface.materialInstance->index].push_back(data);
        }
        else
        {
            context.opaqueSurfaces[surface.materialInstance->index].push_back(data);
        }
    }

    RenderableNode::Draw(topMatrix, context);
}

void Vel::MeshNode::SetSurfacesMaterialData(const MaterialInstance& materialData)
{
    for (auto& surface : mesh->surfaces)
    {
        surface.materialInstance = std::make_shared<MaterialInstance>(materialData);
    }
}

VkImage Vel::RenderableGLTF::errorCheckerboardImage = VK_NULL_HANDLE;
VkImage Vel::RenderableGLTF::defaultNormalsImage = VK_NULL_HANDLE;
VkImage Vel::RenderableGLTF::defaultMetallicRoughnessImage = VK_NULL_HANDLE;

void Vel::RenderableGLTF::Draw(const glm::mat4& topMatrix, DrawContext& context)
{
    for (auto& node : topNodes)
    {
        node->Draw(topMatrix, context);
    }
}

Vel::RenderableGLTF::~RenderableGLTF()
{
    Cleanup();
}

void Vel::RenderableGLTF::Cleanup()
{
    for (auto& [k, v] : meshes)
    {

        allocator->DestroyBuffer(v->meshBuffers.indexBuffer);
        allocator->DestroyBuffer(v->meshBuffers.vertexBuffer);
    }

    for (auto& [k, v] : images)
    {

        if (v.image == errorCheckerboardImage)
        {
            // dont destroy the default images
            continue;
        }
        allocator->DestroyImage(v);
    }

    for (auto& sampler : samplers)
    {
        vkDestroySampler(device, sampler, nullptr);
    }

    auto materialBuffer = materialDataBuffer;

    descriptorPool.Cleanup();

    allocator->DestroyBuffer(materialBuffer);
}
