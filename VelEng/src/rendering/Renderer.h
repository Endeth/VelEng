#pragma once
#include "VulkanTypes.h"
#include "utils/DeletionQueue.h"

#include "Pipeline.h"
#include "Swapchain.h"
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

        uint32_t gPassesCount;
        float gPassesAccTime = 0;
        float gPassesAverage = 0;
    };

    struct Lights
    {
        LightData lights;
        AllocatedBuffer lightsDataBuffer;
        AllocatedBuffer pointLightsBuffer;

        PointLight* pointLightsGPUData; //Used for position update

    };

    class Renderer
    {
    public:
        void Init(SDL_Window* sdlWindow, const VkExtent2D& windowExtent);

        void HandleSDLEvent(SDL_Event* sdlEvent);
        void Draw();

        GPUAllocator* GetAllocator(){ return &gpuAllocator; }
        const DeferredRenderer& GetDeferredRenderer() const { return deferred; }

        AllocatedImage whiteImage;
        AllocatedImage defaultNormalsImage;
        AllocatedImage defaultMetallicRoughnessImage;
        AllocatedImage errorCheckerboardImage;
        VkSampler defaultSamplerLinear;

        void Cleanup();

    private:
        SDL_Window* window;
        bool isInitialized = false;

        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkSurfaceKHR surface;
        VkQueue graphicsQueue;
        uint32_t graphicsQueueFamily;

        Swapchain swapchain;
        //TODO swapchain handler
        //VkSwapchainKHR swapchain;
        //VkFormat swapchainImageFormat;
        //std::vector<VkImage> swapchainImages;
        //std::vector<VkImageView> swapchainImageViews;
        //VkExtent2D swapchainExtent;
        //bool resizeRequested = false;


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
            VkFence renderFence;

            DescriptorAllocatorDynamic frameDescriptors;
            DeletionQueue frameCleanupQueue;
            DeletionQueue cleanupQueue;
        };
        FrameData frames[FRAME_DATA_SIZE];

        MeshLoader meshLoader;

        DeferredRenderer deferred;
        DrawContext mainDrawContext;
        VkExtent2D drawExtent;
        float renderScale = 1.f;

        Camera mainCamera;
        SceneCameraData sceneData;
        VkDescriptorSetLayout sceneCameraDataDescriptorLayout;
        VkDescriptorSet sceneCameraDataDescriptorSet;

        float testRoughness = 0.1f;
        float testMetallic = 0.0f;

        //Performance
        RendererStats stats;

        Vel::Imgui vImgui;

        DeletionQueue delQueue;

        //Debug
        VkDebugUtilsMessengerEXT debugMessenger;
        uint32_t imageToPresent = 0;

        void CreateCommands();
        void CreateSyncStructures();
        void CreateAllocator();
        void CreateCameraDescriptors();
        void InitDeferred();
        void InitImgui();

        void UpdateScene();
        void UpdateActors();
        void UpdateGlobalLighting();
        void UpdateCamera();
        void UpdateGlobalDescriptors();

        void PreparePresentableImage(VkCommandBuffer cmd);

        void PrepareImguiFrame();
        void DrawImgui(VkCommandBuffer cmdBuffer, VkImage drawImage, VkImageView drawImageView, VkImageLayout srcLayout, VkImageLayout dstLayout);

        FrameData& GetCurrentFrame(){ return frames[frameNumber % FRAME_DATA_SIZE]; }

        //Testing
        std::unordered_map<std::string, std::shared_ptr<RenderableGLTF>> loadedScenes;
        Lights testLights;
        glm::vec4 light1Pos;
        glm::vec4 light2Pos;

        void InitTestTextures();
        void InitTestData();
        void InitTestLightData();
        GPUMeshBuffers CreateRectangle();
    };
}
