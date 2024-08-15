#pragma once
#include "VulkanTypes.h"
#include "GPUAllocator.h"

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>

#define GET_TEXTURE_PATH(name) TEXTURE_PATH name

namespace Vel
{
    class STBImage
    {
    public:
        STBImage(const std::filesystem::path& imagePath);
        ~STBImage();

        bool HasImage() { return imageData != nullptr; }
        unsigned char* GetImageData() { return imageData; }
        VkExtent3D GetSize();
    private:
        STBImage(STBImage& other) = delete;
        int width;
        int height;
        int channels;

        unsigned char* imageData;
    };

    std::optional<AllocatedImage> LoadGltfAssetImage(GPUAllocator& allocator, fastgltf::Asset& asset, fastgltf::Image& image, const std::filesystem::path& parentPath);
}
