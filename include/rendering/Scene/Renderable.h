#pragma once

#include "Rendering/VulkanTypes.h"

#include "Rendering/Buffers/Buffers.h"

#include "Rendering/Assets/GLTFObjectLoader.h"

#include "Rendering/RenderPasses/GLTFMaterialPass.h"

namespace Vel
{
    struct RenderData
    {
        uint32_t indexCount;
        uint32_t firstIndex;
        VkBuffer indexBuffer;

        MaterialInstance* materialData;

        glm::mat4 transform;
        VkDeviceAddress vertexBufferAddress;
    };

    struct DrawContext
    {
        DrawContext() = default;
        DrawContext(const size_t materialsInstances)
        {
            opaqueSurfaces.resize(materialsInstances);
            transparentSurfaces.resize(materialsInstances);
        }
        DrawContext(DrawContext& other) = delete;
        DrawContext(DrawContext&& other)
        {
            opaqueSurfaces = std::move(other.opaqueSurfaces);
            transparentSurfaces = std::move(other.transparentSurfaces);
        }

        void operator=(DrawContext&& other)
        {
            opaqueSurfaces = std::move(other.opaqueSurfaces);
            transparentSurfaces = std::move(other.transparentSurfaces);
        }

        void Clear()
        {
            for (auto& materialSurfaces : opaqueSurfaces)
            {
                materialSurfaces.clear();
            }
            for (auto& materialSurfaces : transparentSurfaces)
            {
                materialSurfaces.clear();
            }
        }

        std::vector<std::vector<RenderData>> opaqueSurfaces;
        std::vector<std::vector<RenderData>> transparentSurfaces;
        //std::vector<std::vector<RenderData>> emissiveSurfaces;
    };

    struct ShadowContext
    {
        std::vector<RenderData> surfaces;
    };

    class IRenderable
    {
    public:
        virtual void Draw(const glm::mat4& transformation, DrawContext& context) = 0;
    };

    class RenderableNode : public IRenderable
    {
    public:
        void SetLocalTransform(const glm::mat4& matrix);
        void SetWorldTransform(const glm::mat4& matrix);
        void RefreshTransform(const glm::mat4& parentMatrix);
        virtual void Draw(const glm::mat4& transformation, DrawContext& context) override;

    //protected:
        std::weak_ptr<RenderableNode> parent;
        std::vector<std::shared_ptr<RenderableNode>> children;

        glm::mat4 localTransform;
        glm::mat4 worldTransform;
    };

    struct MeshAsset;
    class MeshNode : public RenderableNode
    {
    public:
        MeshNode(){};
        MeshNode(const std::shared_ptr<MeshAsset>& m);
        virtual void Draw(const glm::mat4& transformation, DrawContext& context) override;
        void SetSurfacesMaterialData(const MaterialInstance& materialData);

    //protected:
        std::shared_ptr<MeshAsset> mesh;
    };

    class RenderableGLTF : public IRenderable
    {
    public:
        std::unordered_map<std::string, std::shared_ptr<MeshAsset>> meshes;
        std::unordered_map<std::string, std::shared_ptr<RenderableNode>> nodes;
        std::unordered_map<size_t, AllocatableImage> images; //TODO vector, just to destroy images
        std::unordered_map<std::string, std::shared_ptr<MaterialInstance>> materials;

        std::vector<std::shared_ptr<RenderableNode>> topNodes;
        std::vector<VkSampler> samplers;

        DescriptorAllocatorDynamic descriptorPool;

        AllocatableBuffer materialDataBuffer;

        virtual void Draw(const glm::mat4& topMatrix, DrawContext& context);
        ~RenderableGLTF();

        GPUAllocator* allocator; //TODO temp until asset manager
        VkDevice device; //TODO temp until asset manager
        static VkImage errorCheckerboardImage; //TODO temp until asset manager
        static VkImage defaultNormalsImage; //TODO temp until asset manager
        static VkImage defaultMetallicRoughnessImage; //TODO temp until asset manager

    private:
        void Cleanup();
    };
}
