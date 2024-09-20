#pragma once

#include "Rendering/VulkanTypes.h"

#include "Rendering/Buffers/Buffers.h"
#include "Rendering/Buffers/GPUAllocator.h"

#include "Rendering/Scene/Camera.h"

namespace Vel
{
    constexpr uint32_t MAX_LIGHTS = 10;

    enum LightType : uint32_t
    {
        DIRECTIONAL = 0,
        POINT = 1
    };

    struct GPULightsUniformBufferData
    {
        LightType type;
        glm::mat4 viewProj;
        glm::vec4 position;
    };

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
        AllocatableImage shadowMap;
        AllocatableBuffer gpuViewProjData;

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
