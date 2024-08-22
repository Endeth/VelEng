#pragma once
#include "VulkanTypes.h"
#include <deque>

#define GET_SHADER_PATH(name) SHADERS_PATH name ".spv"
namespace Vel
{
    class DescriptorLayoutBuilder
    {
    public:
        void AddBinding(uint32_t binding, VkDescriptorType type);
        void Clear();
        VkDescriptorSetLayout Build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);

    private:
        std::vector<VkDescriptorSetLayoutBinding> bindings;
    };

    struct DescriptorPoolSizeRatio
    {
        VkDescriptorType type;
        float ratio;
    };

    class DescriptorAllocator
    {
    public:
        void InitPool(VkDevice dev, uint32_t maxSets, std::span<DescriptorPoolSizeRatio> poolRatios);
        void ClearDescriptors();
        void Cleanup();

        VkDescriptorSet Allocate(VkDescriptorSetLayout layout);
    private:
        VkDevice device;
        VkDescriptorPool pool;
    };

    class DescriptorAllocatorDynamic
    {
    public:
        void InitPool(VkDevice vkDevice, uint32_t maxSets, std::span<DescriptorPoolSizeRatio> poolRatios);
        void ClearPools();
        void Cleanup();

        VkDescriptorSet Allocate(VkDescriptorSetLayout layout);
    private:
        VkDevice device;

        VkDescriptorPool GetPool();
        VkDescriptorPool CreatePool(uint32_t setCount, std::span<DescriptorPoolSizeRatio> poolRatios);

        std::vector<DescriptorPoolSizeRatio> ratios;
        std::vector<VkDescriptorPool> fullPools;
        std::vector<VkDescriptorPool> readyPools;
        uint32_t setsPerPool;
    };

    class DescriptorWriter
    {
    public:
        void WriteImageSampler(uint32_t binding, VkImageView imageView, VkSampler sampler, VkImageLayout layout, VkDescriptorType type);
        void WriteImages(uint32_t binding, VkImageView* imageViews, VkImageLayout imagesLayout, VkDescriptorType type, uint32_t descriptorCount);
        void WriteSampler(uint32_t binding, VkSampler sampler);
        void WriteBuffer(uint32_t binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type);

        void Clear();
        void UpdateSet(VkDevice device, VkDescriptorSet set);

    private:
        std::deque<VkDescriptorImageInfo> imageInfos;
        std::deque<VkDescriptorBufferInfo> bufferInfos;
        std::vector<VkWriteDescriptorSet> writes;
    };
}
