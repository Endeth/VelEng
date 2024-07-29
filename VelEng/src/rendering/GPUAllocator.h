#pragma once
#include "VulkanTypes.h"

namespace Vel
{
    class ImmediateSubmitter
    {
    public:
        void Init(VkDevice dev, uint32_t graphicsQueueFamily, VkQueue targetQueue);
        void Cleanup();

        void Submit(std::function<void(VkCommandBuffer cmd)>&& function);
    private:
        VkDevice device;
        VkQueue queue;

        VkFence immediateFence;
        VkCommandBuffer immediateCommandBuffer;
        VkCommandPool immediateCommandPool;
    };

    class GPUAllocator
    {
    public:
        void Init(VkDevice dev, VmaAllocatorCreateInfo allocatorCreateInfo, uint32_t graphicsQueueFamily, VkQueue targetQueue);
        void Cleanup();

        AllocatedBuffer CreateBuffer(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        void DestroyBuffer(const AllocatedBuffer& buffer);

        VkImageCreateInfo BuildImageCreateInfo(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage);
        VkImageViewCreateInfo BuildImageViewCreateInfo(VkFormat format, VkImageAspectFlags usage, VkImage image);
        AllocatedImage CreateImage(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage, bool mipmapped);
        AllocatedImage CreateImage(void* data, VkExtent3D extent, VkFormat format, VkImageUsageFlags usage, bool mipmapped);
        GPUMeshBuffers UploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);
        void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
        void DestroyImage(const AllocatedImage& image);
    private:
        VkDevice device;
        VmaAllocator vmaAllocator;

        ImmediateSubmitter submitter;
    };
}
