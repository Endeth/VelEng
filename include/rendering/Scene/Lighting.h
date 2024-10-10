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

    struct Sunlight
    {
        glm::vec3 direction;
        glm::vec3 cameraPosition;
        glm::vec3 color;
        glm::mat4 viewProj;

        VkExtent3D shadowResolution;
        AllocatableImage shadowMap;
        AllocatableBuffer shadowViewProj;

        void InitShadowData(GPUAllocator& allocator, VkExtent3D shadowMapResolution);
        void SetLightData(const glm::vec3& col, const glm::vec3& dir);
        void UpdateCameraPosition(const Camera& mainCamera);
    };

    struct PointLight
    {
        glm::vec4 position;
        glm::vec4 color;
    };

    struct SceneLights
    {
        glm::vec4 ambient;
        Sunlight sunlight;
        std::vector<PointLight> pointLights;
    };
}
