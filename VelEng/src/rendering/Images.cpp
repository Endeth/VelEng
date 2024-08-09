#include "Images.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::optional<Vel::AllocatedImage> Vel::LoadImage(GPUAllocator& allocator, fastgltf::Asset& asset, fastgltf::Image& image, const std::filesystem::path& parentPath)
{
    AllocatedImage newImage = {};

    int width;
    int height;
    int nrChannels;

    auto createImage = [&](unsigned char* data)
    {
        VkExtent3D imageSize{ .width = static_cast<uint32_t>(width), .height = static_cast<uint32_t>(height), .depth = 1 };
        newImage = allocator.CreateImage(data, imageSize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, false);
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
    return std::optional<Vel::AllocatedImage>(newImage);
}
