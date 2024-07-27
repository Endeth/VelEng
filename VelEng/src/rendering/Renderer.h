#pragma once
#include "VulkanTypes.h"
#include "utils/DeletionQueue.h"

#include "Pipeline.h"
#include "GPUAllocator.h"
#include "MeshLoader.h"
#include "Camera.h"
#include "Material.h"
#include "Renderable.h"
#include "Deferred.h"

#include "ui/VelImgui.h"

namespace Vel
{   
    struct RendererStats
    {
        float frametime;
        float sceneUpdateTime;
        float dedicatedMaterialDrawTime;
        float gPassDrawTime;
        float lPassDrawTime;
    };

    struct GPass
    {
        AllocatedImage position;
        AllocatedImage color;
        AllocatedImage normals;

        VkDescriptorSetLayout descriptorLayout;
        //Material
        GPassPipeline pipeline;

        VkSemaphore finishDrawing;
        AllocatedImage defaultNormalMap;
        AllocatedImage defaultSpecularMap;
        VkSampler sampler;

        DescriptorAllocatorDynamic descriptorPool;
        DescriptorWriter writer;

        VkDescriptorSet testSet;

        VkImage GetImage(uint32_t idx)
        {
            switch (idx)
            {
            case 0:
                return position.image;
            case 1:
                return color.image;
            case 2:
                return normals.image;
            }

        }
    };

    struct LPass
    {
        GPUMeshBuffers rect;
        AllocatedImage drawImage;
        AllocatedBuffer lightsDataBuffer;
        AllocatedBuffer pointLightsBuffer;
        VkDeviceAddress pointLightsBufferAddress;
        PointLight* pointLightsGPUData;
        VkSampler sampler;

        VkDescriptorSetLayout gBufferDescriptorLayout;
        VkDescriptorSetLayout lightsDescriptorLayout;
        DescriptorAllocatorDynamic descriptorPool;
        VkDescriptorSet gBufferDescriptorSet;
        VkDescriptorSet lightsDescriptorSet;
        DescriptorWriter writer;
        LPassPipeline pipeline;

        VkSemaphore finishDrawing;
        LightData lights;
    };

    class Renderer
    {
    public:
        void Init(SDL_Window* sdlWindow, const VkExtent2D& windowExtent);

        void HandleSDLEvent(SDL_Event* sdlEvent);
        void Draw();
        void DrawCompute(VkCommandBuffer cmdBuffer);
        void DrawGeometry(VkCommandBuffer cmdBuffer);
        void OnWindowResize();

        VkImageSubresourceRange CreateImageSubresourceRangeAll(VkImageAspectFlags aspect);

        VkSemaphoreSubmitInfo CreateSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
        VkCommandBufferSubmitInfo CreateCommandBufferSubmitInfo(VkCommandBuffer cmdBuffer);
        VkSubmitInfo2 CreateSubmitInfo(VkCommandBufferSubmitInfo& cmdBufferInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo, VkSemaphoreSubmitInfo* signalSemaphoreInfo);

        GPUMeshBuffers UploadMesh(std::span<uint32_t> idices, std::span<Vertex> vertices);
        GPUAllocator* GetAllocator(){ return &gpuAllocator; }

        AllocatedImage whiteImage;
        AllocatedImage errorCheckerboardImage;
        VkSampler defaultSamplerLinear;
        VkSampler defaultSamplerNearest;
        GLTFMetallicRoughness gltfMaterialPipeline;

        void Cleanup();
    private:

        //GlobalStructures
        SDL_Window* window;
        bool isInitialized = false;

        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkSurfaceKHR surface;

        VkSwapchainKHR swapchain;
        VkFormat swapchainImageFormat;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImageViews;
        VkExtent2D swapchainExtent;
        bool resizeRequested = false;

        VkQueue graphicsQueue;
        uint32_t graphicsQueueFamily;

        DeletionQueue delQueue;

        GPUAllocator gpuAllocator;

        //Frame specific data
        int frameNumber = 0;
        constexpr static size_t FRAME_DATA_SIZE = 2;
        struct FrameData
        {
            VkCommandPool commandPool;
            VkCommandBuffer commandBuffer;
            VkCommandBuffer lPassCommands;
            VkSemaphore swapchainSemaphore;
            VkSemaphore renderSemaphore;
            VkFence renderFence;

            DescriptorAllocatorDynamic frameDescriptors;
            DeletionQueue frameCleanupQueue;
            DeletionQueue cleanupQueue;
        };
        FrameData frames[FRAME_DATA_SIZE];
        AllocatedImage drawImage;
        AllocatedImage depthImage;
        VkExtent2D drawExtent;
        float renderScale = 1.f;

        //Pipeline
        DescriptorAllocatorDynamic globalDescriptorAllocator;
        VkDescriptorSetLayout drawImageDescriptorLayout;
        VkDescriptorSet drawImageDescriptors;

        Camera mainCamera;
        SceneCameraData sceneData;
        VkDescriptorSetLayout sceneCameraDataDescriptorLayout;
        VkDescriptorSet sceneCameraDataDescriptorSet;

        VulkanComputePipeline gradientPipeline;

        DrawContext mainDrawContext;

        //Deferred rendering
        GPass gPassData;
        LPass lPassData;

        //Performance
        RendererStats stats;

        Vel::Imgui vImgui;

        //Testing
        uint32_t blitTarget = 0;
        std::unordered_map<std::string, std::shared_ptr<RenderableNode>> loadedNodes;
        std::unordered_map<std::string, std::shared_ptr<RenderableGLTF>> loadedScenes;

        void InitDeferred();
        void InitTestTextures();
        void InitTestData();
        GPUMeshBuffers CreateRectangle();

        //Debug
        VkDebugUtilsMessengerEXT debugMessenger;

        void CreateSwapchain(uint32_t width, uint32_t height);
        void CreateDrawImage();
        void CreateCommands();
        void CreateSyncStructures();
        void CreateAllocator();
        void CreateDescriptors();
        void CreatePipelines();
        void InitImgui();


        VkRenderingAttachmentInfo BuildColorAttachmentInfo(VkImageView imageView);
        VkRenderingAttachmentInfo BuildGPassAttachmentInfo(VkImageView imageView);
        VkRenderingAttachmentInfo BuildLPassAttachmentInfo(VkImageView imageView);
        VkRenderingAttachmentInfo BuildDepthAttachmentInfo();
        VkRenderingInfo BuildGeometryDrawRenderInfo(VkRenderingAttachmentInfo* color, uint32_t colorAttachmentsCount, VkRenderingAttachmentInfo* depth);
        VkViewport BuildGeometryDrawViewport();
        VkRect2D BuildGeometryDrawScissors();

        void UpdateScene();
        void UpdateGlobalLighting();
        void UpdateCamera();
        void UpdateGlobalDescriptors();

        void DestroySwapchain();

        FrameData& GetCurrentFrame();

        void ClearBackground(VkCommandBuffer cmdBuffer);
        void BlitImage(VkCommandBuffer cmdBuffer, VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize);
    };
}
