#pragma once

#include <memory>
#include <string>
#include <vector>
#include <array>
#include <functional>
#include <chrono>
#include <thread>
#include <span>
#include <map>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include "fmt/format.h"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "VkBootstrap.h"

#include "config/VelEngConfig.h"

#define VK_CHECK(x)                                                          \
    do {                                                                     \
        VkResult err = x;                                                    \
        if (err) {                                                           \
            fmt::println("Detected Vulkan error: {}", string_VkResult(err)); \
            abort();                                                         \
        }                                                                    \
    } while (0)

namespace Vel
{
    constexpr static uint32_t FRAME_TIMEOUT = 1000000000;
    constexpr static uint32_t RENDER_THREADS_COUNT = 2;

    struct Vertex
    {
        glm::vec3 position;
        float uv_x;
        glm::vec3 normal;
        float uv_y;
        glm::vec4 tangent;
    };

    struct GPUDrawPushConstants
    {
        glm::mat4 worldMatrix;
        VkDeviceAddress vertexBuffer;
    };

    struct SkyboxPushConstants
    {
        glm::mat4 inverseViewProjection;
        glm::vec4 cameraPosition;
    };

    struct SceneCameraData
    {
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 viewProjection;
        glm::mat4 invViewProjection;
        glm::vec4 position;
        glm::vec4 testData;
    };

    struct AllocatedImage
    {
        VkImage image;
        VkImageView imageView;
        VmaAllocation allocation;
        VkExtent3D imageExtent;
        VkFormat imageFormat;
    };

    struct AllocatedBuffer
    {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo info;
    };

    struct GPUMeshBuffers
    {
        AllocatedBuffer indexBuffer;
        AllocatedBuffer vertexBuffer;
        VkDeviceAddress vertexBufferAddress;
    };

    // General work queue
    //  -- prepare cpu data and send (unlocks on job done)
    //  -- prepare light data and send (unlocks on job done)
    //  -- set initial images layout (semaphore signal)
    //  -- maybe get swapchain?

    // Main context work queue (adds work to record gpass and unlocks it on no more work)
    // |
    // v
    // Record gpass work commands queue (unlocks on no more work)
    // |
    // | -- waits for initial img layout semaphore on GPU
    // v
    // Queue gpass callback (unlocks on finish) or work commands queue (unlocks on no more work)

    // Shadow context work queue (adds work to record shadows and unlocks it on no more work)
    // |
    // v
    // Record shadows commands work queue (unlocks on no more work)
    // |
    // | -- waits for initial img layout semaphore on GPU
    // v
    // Queue shadows callback (unlocks on finish) or work commands queue (unlocks on no more work)

    // LPass + blit record and present queue
    // -- might happen only after camera and lights, queue waits for semaphores gpass shadows skybox and imgs layout
    // -- blit waits for getting swapchain
    // -- present waits for swapchain and lpass semaphore

    enum RenderQueueType : uint8_t
    {
        GENERAL,
        PREPARED_FRAME_HANDLER,
        MAIN_CONTEXT,
        MAIN_COMMANDS_RECORD,
        SHADOW_CONTEXT,
        SHADOW_COMMANDS_RECORD,
        LPASS_COMMANDS_RECORD
    };

    enum RenderStages : uint8_t
    {
        NONE,
        PREPARE_CPU_DATA,
        PREPARE_MAIN_CONTEXT,
        PREPARE_SHADOW_CONTEXT,
        SET_INITIAL_IMAGES_LAYOUT,
        UPDATE_GPU_CAMERA_DATA,
        UPDATE_GPU_LIGHT_DATA,
        FRAME_PREPARED,
        RECORD_GPASS,
        RECORD_SHADOWS,
        QUEUE_GPASS,
        QUEUE_SHADOWS,
        RECORD_QUEUE_SKYBOX,
        RECORD_QUEUE_LPASS,
        GET_SWAPCHAIN_IMAGE,
        //SWAPCHAIN_IMAGE_BLIT,
        SWAPCHAIN_PRESENT
    };

    enum class MaterialPass : uint8_t
    {
        MainColor,
        Transparent,
        Emissive,
        Other
    };

    struct MaterialPipeline
    {
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
    };

    struct MaterialResources
    {
        AllocatedImage colorImage;
        VkSampler colorSampler; //Use one sampler;
        AllocatedImage normalsImage;
        VkSampler normalsSampler;
        AllocatedImage metallicRoughnessImage;
        VkSampler metallicRoughnessSampler;
        AllocatedImage emissiveImage; //separate?
        VkSampler emissiveSampler;
        VkBuffer dataBuffer; //Model materials constants buffer
        uint32_t dataBufferOffset; //Material offset
    };

    struct MaterialConstants
    {
        glm::vec4 color;
        glm::vec4 metallicRoughnessFactor;

        //padding and uniform buffers
        glm::vec4 extra[14];
    };

    struct MaterialInstance
    {
        static uint32_t instancesCount;

        MaterialInstance()
        {
            index = instancesCount++;
            //pipeline = nullptr;
            descriptorSet = VK_NULL_HANDLE;
            passType = MaterialPass::Other;
        }

        uint32_t index;
        //MaterialPipeline* pipeline; //Fixed pipelines right now
        VkDescriptorSet descriptorSet;
        MaterialPass passType;
    };

    struct GeoSurface
    {
        uint32_t startIndex;
        uint32_t count;
        std::shared_ptr<MaterialInstance> materialInstance;
    };

    struct MeshAsset
    {
        std::string name;

        std::vector<GeoSurface> surfaces;
        GPUMeshBuffers meshBuffers;
    };

    struct LightData
    {
        glm::vec4 ambient;
        glm::vec4 sunlightDirection;
        glm::vec4 sunlightColor;
        glm::mat4 sunlightViewProj;
        uint32_t sunlightShadowMapID;

        uint32_t pointLightsCount;
        VkDeviceAddress pointLightBuffer;
    };

    struct PointLight
    {
        glm::vec4 position;
        glm::vec4 color;
    };
}
