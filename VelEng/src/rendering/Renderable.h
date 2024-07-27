#pragma once
#include "VulkanTypes.h"
#include "Material.h"
#include "MeshLoader.h"

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
        //std::vector<RenderData> opaqueSurfaces;
        //std::multimap<size_t, RenderData> opaqueSurfaces;
        std::vector<std::vector<RenderData>> opaqueSurfaces;
        //std::vector<RenderData> transparentSurfaces;
        //std::multimap<size_t, RenderData> transparentSurfaces;
        std::vector<std::vector<RenderData>> transparentSurfaces;
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
        std::unordered_map<std::string, AllocatedImage> images;
        std::unordered_map<std::string, std::shared_ptr<MaterialInstance>> materials;

        std::vector<std::shared_ptr<RenderableNode>> topNodes;
        std::vector<VkSampler> samplers;

        DescriptorAllocatorDynamic descriptorPool;

        AllocatedBuffer materialDataBuffer;

        virtual void Draw(const glm::mat4& topMatrix, DrawContext& context);
        ~RenderableGLTF();

        GPUAllocator* allocator; //TODO temp until asset manager
        VkDevice device; //TODO temp until asset manager
        static VkImage errorCheckerboardImage; //TODO temp until asset manager

    private:
        void Cleanup();
    };
}
