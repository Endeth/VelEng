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

    struct Lights
    {
        LightData lights;
        AllocatedBuffer lightsDataBuffer;
        AllocatedBuffer pointLightsBuffer;
        VkDeviceAddress pointLightsBufferAddress;
        PointLight* pointLightsGPUData;
    };

    class Renderer
    {
    public:
        constexpr static uint32_t FRAME_TIMEOUT = 1000000000;

        void Init(SDL_Window* sdlWindow, const VkExtent2D& windowExtent);

        void HandleSDLEvent(SDL_Event* sdlEvent);
        void Draw();
        void OnWindowResize();

        GPUAllocator* GetAllocator(){ return &gpuAllocator; }

        AllocatedImage whiteImage;
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

        //TODO swapchain handler
        VkSwapchainKHR swapchain;
        VkFormat swapchainImageFormat;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImageViews;
        VkExtent2D swapchainExtent;
        bool resizeRequested = false;


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

        DeferredRenderer deferred;
        DrawContext mainDrawContext;
        VkExtent2D drawExtent;
        float renderScale = 1.f;

        Camera mainCamera;
        SceneCameraData sceneData;
        VkDescriptorSetLayout sceneCameraDataDescriptorLayout;
        VkDescriptorSet sceneCameraDataDescriptorSet;

        //Performance
        RendererStats stats;

        Vel::Imgui vImgui;

        DeletionQueue delQueue;

        //Debug
        VkDebugUtilsMessengerEXT debugMessenger;

        void CreateSwapchain(uint32_t width, uint32_t height);
        void CreateCommands();
        void CreateSyncStructures();
        void CreateAllocator();
        void CreateCameraDescriptors();
        void InitDeferred();
        void InitImgui();

        void UpdateScene();
        void UpdateGlobalLighting();
        void UpdateCamera();
        void UpdateGlobalDescriptors();

        void DrawImgui(VkCommandBuffer cmdBuffer, VkImage drawImage, VkImageView drawImageView, VkImageLayout srcLayout, VkImageLayout dstLayout);

        void DestroySwapchain();

        FrameData& GetCurrentFrame(){ return frames[frameNumber % FRAME_DATA_SIZE]; }

        //Testing
        std::unordered_map<std::string, std::shared_ptr<RenderableGLTF>> loadedScenes;
        Lights testLights;

        void InitTestTextures();
        void InitTestData();
        void InitTestLightData();
        GPUMeshBuffers CreateRectangle();
    };
}
