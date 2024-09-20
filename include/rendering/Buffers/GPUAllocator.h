#pragma once

#include "Rendering/VulkanTypes.h"

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

    class GPUAllocator //TODO extract Image
    {
    public:
        static constexpr uint32_t cubeTextureLayers = 6;

        void Init(VkDevice dev, VmaAllocatorCreateInfo allocatorCreateInfo, uint32_t graphicsQueueFamily, VkQueue targetQueue);
        void Cleanup();

        AllocatedBuffer CreateBuffer(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        void DestroyBuffer(const AllocatedBuffer& buffer);
        AllocatedBuffer& GetStagingBuffer();

        VkImageCreateInfo Create2DImageCreateInfo(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage);
        VkImageCreateInfo CreateCubeImageCreateInfo(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage);
        VkImageViewCreateInfo Create2DImageViewCreateInfo(VkFormat format, VkImageAspectFlags usage, VkImage image);
        VkImageViewCreateInfo CreateCubeImageViewCreateInfo(VkFormat format, VkImage image);
        VkBufferImageCopy CreateBufferImageCopy(VkExtent3D extent, uint32_t layersCount);

        void AllocateImage(AllocatedImage& image);
        AllocatedImage CreateAllocatedImage(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage);
        AllocatedImage CreateImageFromData(void* data, VkExtent3D extent, VkFormat format, VkImageUsageFlags usage);
        AllocatedImage CreateAllocatedCubeImage(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage);
        AllocatedImage CreateCubeImageFromData(std::array<unsigned char*, cubeTextureLayers> data, VkExtent3D extent, VkFormat format, VkImageUsageFlags usage);

        GPUMeshBuffers UploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);
        void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
        void DestroyImage(const AllocatedImage& image);
    private:
        VkDevice device;
        VmaAllocator vmaAllocator;

        ImmediateSubmitter submitter;

        AllocatedBuffer stagingBuffer;
    };
}
