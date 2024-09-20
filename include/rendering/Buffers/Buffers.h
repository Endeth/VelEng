#pragma once

#include "Rendering/VulkanTypes.h"

namespace Vel
{
    struct AllocatableImage
    {
        VkImage image;
        VkImageView imageView;
        VkImageUsageFlags usageFlags;
        VmaAllocation allocation;
        VkExtent3D extent;
        VkFormat format;
    };

    struct AllocatableBuffer
    {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo info;
    };

    struct GPUMeshBuffers
    {
        AllocatableBuffer indexBuffer;
        AllocatableBuffer vertexBuffer;
        VkDeviceAddress vertexBufferAddress;
    };
}
