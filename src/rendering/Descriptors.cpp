#include "Rendering/Descriptors.h"

#include <fstream>

void Vel::DescriptorLayoutBuilder::AddBinding(uint32_t binding, VkDescriptorType type)
{
    VkDescriptorSetLayoutBinding bind {
        .binding = binding,
        .descriptorType = type,
        .descriptorCount = 1
    };


    bindings.push_back(bind);
}

void Vel::DescriptorLayoutBuilder::Clear()
{
    bindings.clear();
}

VkDescriptorSetLayout Vel::DescriptorLayoutBuilder::Build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext, VkDescriptorSetLayoutCreateFlags flags)
{
    for (auto& binding : bindings)
    {
        binding.stageFlags |= shaderStages;
    }

    VkDescriptorSetLayoutCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = pNext,

        .flags = flags,
        .bindingCount = (uint32_t)bindings.size(),
        .pBindings = bindings.data()
    };

    VkDescriptorSetLayout setLayout;
    VK_CHECK(vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &setLayout));
    return setLayout;
}

void Vel::DescriptorAllocator::InitPool(VkDevice dev, uint32_t maxSets, std::span<DescriptorPoolSizeRatio> poolRatios)
{
    device = dev;

    std::vector<VkDescriptorPoolSize> poolSizes;
    for (const DescriptorPoolSizeRatio& ratio : poolRatios)
    {
        poolSizes.push_back(VkDescriptorPoolSize{
            .type = ratio.type, 
            .descriptorCount = uint32_t(ratio.ratio * maxSets)
        });
    }

    VkDescriptorPoolCreateInfo poolInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = maxSets,
        .poolSizeCount = (uint32_t)poolSizes.size(),
        .pPoolSizes = poolSizes.data()
    };

    VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool));
}

void Vel::DescriptorAllocator::ClearDescriptors()
{
    vkResetDescriptorPool(device, pool, 0);
}

void Vel::DescriptorAllocator::Cleanup()
{
    vkDestroyDescriptorPool(device, pool, nullptr);
}

VkDescriptorSet Vel::DescriptorAllocator::Allocate(VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &layout
    };

    VkDescriptorSet set;
    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &set));

    return set;
}

void Vel::DescriptorAllocatorDynamic::InitPool(VkDevice vkDevice, uint32_t maxSets, std::span<DescriptorPoolSizeRatio> poolRatios)
{
    device = vkDevice;

    ratios.clear();

    for (auto ratio : poolRatios)
    {
        ratios.push_back(ratio);
    }

    VkDescriptorPool newPool = CreatePool(maxSets, poolRatios);

    setsPerPool = static_cast<uint32_t>(maxSets * 1.5f);
    readyPools.push_back(newPool);
}

void Vel::DescriptorAllocatorDynamic::ClearPools()
{
    for (auto pool : readyPools)
    {
        vkResetDescriptorPool(device, pool, 0);
    }

    for (auto pool : fullPools)
    {
        vkResetDescriptorPool(device, pool, 0);
        readyPools.push_back(pool);
    }
    fullPools.clear();
}

void Vel::DescriptorAllocatorDynamic::Cleanup()
{
    for (auto pool : readyPools)
    {
        vkDestroyDescriptorPool(device, pool, nullptr);
    }
    readyPools.clear();

    for (auto pool : fullPools)
    {
        vkDestroyDescriptorPool(device, pool, nullptr);
    }
    fullPools.clear();
}

VkDescriptorSet Vel::DescriptorAllocatorDynamic::Allocate(VkDescriptorSetLayout layout)
{
    VkDescriptorPool poolToUse = GetPool();

    VkDescriptorSetAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = poolToUse,
        .descriptorSetCount = 1,
        .pSetLayouts = &layout
    };

    VkDescriptorSet descriptorSet;
    VkResult allocResult = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);

    if (allocResult == VK_ERROR_OUT_OF_POOL_MEMORY || allocResult == VK_ERROR_FRAGMENTED_POOL)
    {
        fullPools.push_back(poolToUse);

        poolToUse = GetPool();
        allocInfo.descriptorPool = poolToUse;

        VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));
    }

    readyPools.push_back(poolToUse);
    return descriptorSet;
}

VkDescriptorPool Vel::DescriptorAllocatorDynamic::GetPool()
{
    VkDescriptorPool pool;
    if (readyPools.size() > 0)
    {
        pool = readyPools.back();
        readyPools.pop_back();
    }
    else
    {
        pool = CreatePool(setsPerPool, ratios);
        setsPerPool *= 1.5f;

        if (setsPerPool > 4092)
            setsPerPool = 4092;
    }

    return pool;
}

VkDescriptorPool Vel::DescriptorAllocatorDynamic::CreatePool(uint32_t setCount, std::span<DescriptorPoolSizeRatio> poolRatios)
{
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (auto ratio : poolRatios)
    {
        poolSizes.push_back(VkDescriptorPoolSize{
            .type = ratio.type,
            .descriptorCount = uint32_t(ratio.ratio * setCount)
        });
    }

    VkDescriptorPoolCreateInfo poolCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = setCount,
        .poolSizeCount = (uint32_t)poolSizes.size(),
        .pPoolSizes = poolSizes.data()
    };

    VkDescriptorPool newPool;
    vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &newPool);

    return newPool;
}

void Vel::DescriptorWriter::WriteImageSampler(uint32_t binding, VkImageView imageView, VkSampler sampler, VkImageLayout layout, VkDescriptorType type)
{
    VkDescriptorImageInfo& info = imageInfos.emplace_back(VkDescriptorImageInfo{
        .sampler = sampler,
        .imageView = imageView,
        .imageLayout = layout
    });

    VkWriteDescriptorSet write {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,

        .dstSet = VK_NULL_HANDLE,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = type,
        .pImageInfo = &info
    };

    writes.push_back(write);
}

void Vel::DescriptorWriter::WriteImages(uint32_t binding, VkImageView* imageViews, VkImageLayout imagesLayout, VkDescriptorType type, uint32_t descriptorCount)
{
    VkDescriptorImageInfo* infos = nullptr;

    for (uint32_t i = 0; i < descriptorCount; ++i)
    {
        VkDescriptorImageInfo& info = imageInfos.emplace_back(VkDescriptorImageInfo{
            .imageView = imageViews[i],
            .imageLayout = imagesLayout
        });

        if (!infos)
            infos = &info;
    }

    VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,

        .dstSet = VK_NULL_HANDLE,
        .dstBinding = binding,
        .descriptorCount = descriptorCount,
        .descriptorType = type,
        .pImageInfo = infos
    };

    writes.push_back(write);
}

void Vel::DescriptorWriter::WriteSampler(uint32_t binding, VkSampler sampler)
{
    VkDescriptorImageInfo& info = imageInfos.emplace_back(VkDescriptorImageInfo{
        .sampler = sampler,
     });

    VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,

        .dstSet = VK_NULL_HANDLE,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .pImageInfo = &info
    };

    writes.push_back(write);
}

void Vel::DescriptorWriter::WriteBuffer(uint32_t binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type)
{
    VkDescriptorBufferInfo& info = bufferInfos.emplace_back(VkDescriptorBufferInfo{
        .buffer = buffer,
        .offset = offset,
        .range = size
    });

    VkWriteDescriptorSet write {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,

        .dstSet = VK_NULL_HANDLE,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = type,
        .pBufferInfo = &info
    };

    writes.push_back(write);
}

void Vel::DescriptorWriter::Clear()
{
    imageInfos.clear();
    bufferInfos.clear();
    writes.clear();
}

void Vel::DescriptorWriter::UpdateSet(VkDevice device, VkDescriptorSet set)
{
    for (VkWriteDescriptorSet& write : writes)
        write.dstSet = set;

    vkUpdateDescriptorSets(device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}
