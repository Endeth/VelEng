#include "Rendering/Assets/Images.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::optional<Vel::AllocatableImage> Vel::LoadGltfAssetImage(GPUAllocator& allocator, fastgltf::Asset& asset, fastgltf::Image& image, const std::filesystem::path& parentPath)
{
    AllocatableImage newImage = {};

    int width;
    int height;
    int nrChannels;

    auto createImage = [&](unsigned char* data)
    {
        VkExtent3D imageSize{ .width = static_cast<uint32_t>(width), .height = static_cast<uint32_t>(height), .depth = 1 };
        newImage = allocator.CreateImageFromData(data, imageSize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
        stbi_image_free(data);
    };

    std::visit(
        fastgltf::visitor {
            [](auto& arg) {},
            [&](fastgltf::sources::URI& filePath)
            {
                assert(filePath.fileByteOffset == 0);
                assert(filePath.uri.isLocalPath());

                const std::string path(filePath.uri.path().begin(), filePath.uri.path().end());
                unsigned char* data = stbi_load((parentPath.string() + "//" + path).c_str(), &width, &height, &nrChannels, 4);
                if (data)
                {
                    createImage(data);
                }
            },
            [&](fastgltf::sources::Vector& vector)
            {
                unsigned char* data = stbi_load_from_memory(vector.bytes.data(), static_cast<int>(vector.bytes.size()),
                    &width, &height, &nrChannels, 4);
                if (data)
                {
                    createImage(data);
                }
            },
            [&](fastgltf::sources::BufferView& view)
            {
                auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                auto& buffer = asset.buffers[bufferView.bufferIndex];

                // We only care about VectorWithMime here, because we specify LoadExternalBuffers,
                // meaning all buffers are already loaded into a vector.
                std::visit(
                    fastgltf::visitor {
                               [](auto& arg) {},
                               [&](fastgltf::sources::Vector& vector)
                               {
                                   unsigned char* data = stbi_load_from_memory(vector.bytes.data() + bufferView.byteOffset,
                                       static_cast<int>(bufferView.byteLength),
                                       &width, &height, &nrChannels, 4);
                                   if (data)
                                   {
                                       createImage(data);
                                   }
                               }
                    },
                    buffer.data);
            }
        },
        image.data
    );

    if (newImage.image == VK_NULL_HANDLE)
    {
        return {};
    }
    else
    {
        return newImage;
    }
    return std::optional<Vel::AllocatableImage>(newImage);
}

Vel::STBImage::STBImage(const std::filesystem::path& imagePath)
{
    imageData = stbi_load(imagePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (imageData == nullptr)
    {
        std::cerr << "Failed to load image: " << imagePath << std::endl;
    }
}

Vel::STBImage::~STBImage()
{
    if (imageData != nullptr)
    {
        stbi_image_free(imageData);
    }
}

VkExtent3D Vel::STBImage::GetSize()
{
    return VkExtent3D{ .width = (uint32_t)width, .height = (uint32_t)height, .depth = 1 };
}
