#pragma once
#include "VulkanTypes.h"
#include "GPUAllocator.h"

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>

namespace Vel
{
    std::optional<AllocatedImage> LoadImage(GPUAllocator& allocator, fastgltf::Asset& asset, fastgltf::Image& image);
}
