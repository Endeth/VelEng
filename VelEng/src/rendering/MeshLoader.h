#pragma once
#include <filesystem>

#include "VulkanTypes.h"
#include "GPUAllocator.h"
#include "Material.h"
#include "Renderable.h"

#define GET_MESH_PATH(name) MESH_PATH name ".glb"

namespace Vel
{
    class Renderer;
    class RenderableGLTF;

    std::optional<std::vector<std::shared_ptr<MeshAsset>>> loadGltfMeshes(Renderer* renderer, const std::filesystem::path& filePath);
    std::optional<std::shared_ptr<RenderableGLTF>> loadGltf(VkDevice device, const std::filesystem::path& filePath, Renderer* renderer);
}
