#include "Rendering/Scene/Lighting.h"

void Vel::Sunlight::InitLightData(const glm::vec3& dir, const glm::vec4& col)
{
    direction = dir;
    color = col;
    position = glm::vec3(0.0f);
}

void Vel::Sunlight::InitShadowData(GPUAllocator& allocator, VkExtent3D shadowMapResolution)
{
    shadowViewProj = allocator.CreateBuffer(sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    shadowResolution = shadowMapResolution;

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        | VK_IMAGE_USAGE_SAMPLED_BIT
        | VK_IMAGE_USAGE_TRANSFER_SRC_BIT //TODO needed?
        | VK_IMAGE_USAGE_TRANSFER_DST_BIT; //TODO remove debug dst_bit
    shadowMap = allocator.CreateAllocatableImage(shadowMapResolution, VK_FORMAT_D32_SFLOAT, usageFlags);
}

void Vel::Sunlight::UpdateCameraPosition(const Camera& mainCamera)
{
    float radius = 10.f;
    float zNear = -100.f;
    float zFar = 100.f;

    position = glm::vec3(mainCamera.GetPosition()) - direction;

    glm::mat4 projection = glm::ortho(-radius, radius, -radius, radius, zNear, zFar);
    projection[1][1] *= -1;

    glm::mat4 view = glm::lookAt(position, position + direction, glm::vec3(0.0f, -1.0f, 0.0f));
    viewProj = projection * view;
    
    memcpy(shadowViewProj.info.pMappedData, &viewProj, sizeof(glm::mat4));
}
