#pragma once
#include <filesystem>

#include <fastgltf/parser.hpp>

#include "Rendering/VulkanTypes.h"

#include "Rendering/Buffers/Buffers.h"
#include "Rendering/Buffers/GPUAllocator.h"

//#include "Rendering/RenderPasses/GLTFMaterialPass.h"
#include "Rendering/Scene/Renderable.h"

#define GET_MESH_PATH(name) MESH_PATH name

namespace Vel
{
    class Renderer;
    class RenderableGLTF;

    struct GeoSurface
    {
        uint32_t startIndex;
        uint32_t count;
        std::shared_ptr<MaterialInstance> materialInstance;
    };

    struct MeshAsset
    {
        std::string name;

        std::vector<GeoSurface> surfaces;
        GPUMeshBuffers meshBuffers;
    };

    // TODO again, more of a factory
    class GLTFObjectLoader
    {
    public:
        void Init(VkDevice dev, Renderer* ren, GPUAllocator* gpuAllocator);
        void Cleanup();
        std::optional<std::shared_ptr<RenderableGLTF>> loadGltf(const std::filesystem::path& filePath);

    private:
        VkDevice device;
        Renderer* renderer;
        GPUAllocator* allocator;
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
