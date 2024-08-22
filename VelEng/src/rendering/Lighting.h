#pragma once
#include "VulkanTypes.h"
#include "GPUAllocator.h"
#include "Camera.h"

namespace Vel
{
    struct SceneLights
    {
        glm::vec4 ambient;
    };

    struct AmbientLight //SSAO info
    {
        glm::vec4 color;
    };

    struct Sunlight
    {
        glm::vec3 direction;
        glm::vec3 position;
        glm::vec4 color;
        glm::mat4 viewProj;

        VkExtent3D shadowResolution;
        AllocatedImage shadowMap;
        AllocatedBuffer gpuViewProjData;

        void InitLightData(const glm::vec3& dir, const glm::vec4& col);
        void InitShadowData(GPUAllocator& allocator, VkExtent3D shadowMapResolution);
        void UpdateCameraPosition(const Camera& mainCamera);
    };

    /*struct PointLight
    {

    };*/

    struct ShadedPointLight
    {

    };
}
