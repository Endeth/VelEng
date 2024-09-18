#pragma once
#include <filesystem>

#include "VulkanTypes.h"
#include "GPUAllocator.h"
#include "Material.h"
#include "Renderable.h"
#include <fastgltf/parser.hpp>

#define GET_MESH_PATH(name) MESH_PATH name

namespace Vel
{
    class Renderer;
    class RenderableGLTF;

    class MeshLoader
    {
    public:
        void Init(VkDevice dev, Renderer* ren);
        void Cleanup();
        std::optional<std::shared_ptr<RenderableGLTF>> loadGltf(const std::filesystem::path& filePath);

    private:
        VkDevice device;
        Renderer* renderer;
        std::filesystem::path parentPath;
        std::vector<std::shared_ptr<MeshAsset>> meshes;
        std::vector<std::shared_ptr<MaterialInstance>> materials;

        bool LoadAsset(const std::filesystem::path& filePath, fastgltf::Asset& gltfAsset);
        void CreateSamplers(RenderableGLTF& sceneData, fastgltf::Asset& gltfAsset);
        void CreateSurfaces(RenderableGLTF& sceneData, fastgltf::Asset& gltfAsset);
        void CreateMaterials(RenderableGLTF& sceneData, fastgltf::Asset& gltfAsset);
        void CreateNodeTree(RenderableGLTF& sceneData, fastgltf::Asset& gltfAsset);
    };
}
