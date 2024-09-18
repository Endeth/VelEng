#pragma once
#include "VulkanTypes.h"
#include "utils/DeletionQueue.h"

#include "Utils/RenderThreadPool.h"
#include "Rendering/Frames.h"
#include "Rendering/Swapchain.h"
#include "Rendering/GPUAllocator.h"
#include "Rendering/MeshLoader.h"
#include "Rendering/Camera.h"
#include "Rendering/Material.h"
#include "Rendering/Lighting.h"
#include "Rendering/Renderable.h"

#include "Rendering/RenderPasses/Pipeline.h"
#include "Rendering/RenderPasses/DeferredPasses.h"
#include "Rendering/RenderPasses/ShadowPass.h"
#include "Rendering/RenderPasses/SkyboxPass.h"

#include "ui/VelImgui.h"

namespace Vel
{   
    struct RendererStats
    {
        float frametime;
        float sceneUpdateTime;
        float contextCreation;
        float gPassDrawTime;
        float lPassDrawTime;

        uint32_t gPassesCount;
        float gPassesAccTime = 0;
        float gPassesAverage = 0;
    };

    struct Lights
    {
        Sunlight sunlight;
        LightData lights;
        AllocatedBuffer lightsDataBuffer;
        AllocatedBuffer pointLightsBuffer;

        PointLight* pointLightsGPUData; //Used for position update

    };

    class Renderer
    {
    public:
        Renderer(); //Only for RenderThread
        ~Renderer();

        //Configure() before Init?

        void Init(SDL_Window* sdlWindow, const VkExtent2D& windowExtent);

        void HandleSDLEvent(SDL_Event* sdlEvent);
        void Draw();

        GPUAllocator* GetAllocator(){ return &gpuAllocator; }
        const DeferredPasses& GetDeferredRenderer() const { return deferredPasses; }

        AllocatedImage whiteImage;
        AllocatedImage defaultNormalsImage;
        AllocatedImage defaultMetallicRoughnessImage;
        AllocatedImage defaultCubeImage;
        AllocatedImage errorCheckerboardImage;
        VkSampler defaultSamplerLinear;

        void Cleanup();

    private:
        SDL_Window* window;
        bool isInitialized = false;
        RenderThreadPool renderThreadPool;


        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkSurfaceKHR surface;
        VkQueue graphicsQueue;
        uint32_t graphicsQueueFamily;
        VkCommandBufferBeginInfo primaryCommandBegin;

        Swapchain swapchain;

        GPUAllocator gpuAllocator;

        //Frame specific data
        uint32_t frameNumber = 0;
        constexpr static size_t FRAME_DATA_SIZE = 2;

        /*
        struct FrameData
        {
            VkCommandPool GetAvailableCommandPool()
            {
                VkCommandPool pool;
                //Should always be able to return pool
                commandPools.TryPop(pool);

                return pool;
            }

            void ReaddCommandPool(VkCommandPool pool)
            {
                commandPools.Push(std::move(pool));
            }
            //Frame info
            uint32_t idx;

            //Work resources
            std::list<RenderWorkQueue> frameWorkQueues;
            TSQueue<VkCommandPool> commandPools;

            //Sync resources
            VkSemaphore swapchainSemaphore;
            VkSemaphore skyboxSemaphore;
            VkSemaphore gPassSemaphore;
            VkSemaphore shadowsSemaphore; //TODO dynamic queue?
            VkSemaphore lPassSemaphore;
            std::atomic<bool> rendering = false;
            VkFence renderFence;

            //Data resources
            AllocatedBuffer sceneCameraDataBuffer;
            VkDescriptorSet sceneCameraDescriptorSet;
            DescriptorAllocatorDynamic frameDescriptors;

            DeletionQueue frameEndCleanup;
            DeletionQueue cleanupQueue;
        };//*/
        FrameData frames[FRAME_DATA_SIZE];

        MeshLoader meshLoader;

        SkyboxPass skyboxPass;
        ShadowPass shadowPass;
        DeferredPasses deferredPasses;
        std::vector<DrawContext> mainDrawContexts;
        std::mutex drawContextMutex;
        std::mutex graphicsQueueMutex;
        //DrawContext mainDrawContext; //TODO let's try to use vector
        VkExtent2D drawExtent;
        float renderScale = 1.f;

        Camera mainCamera;
        SceneCameraData sceneData;
        VkDescriptorSetLayout sceneCameraDataDescriptorLayout;

        //Performance
        RendererStats stats;

        Vel::Imgui vImgui;

        DeletionQueue delQueue;

        //Debug
        VkDebugUtilsMessengerEXT debugMessenger;
        uint32_t imageToPresent = 0;

        void CreateFrameData();
        void CreateCommandsInfo();
        VkCommandBuffer CreateCommandBuffer(VkCommandPool pool, VkCommandBufferLevel level);
        void CreateAllocator();
        void CreateCameraDescriptors();

        void InitSkyboxPass();
        void InitShadowPass();
        void InitDeferred();
        void InitImgui();

        void UpdateScene();
        void UpdateActors();
        void UpdateGlobalLighting();
        void UpdateCamera();
        void UpdateFrameDescriptors();

        void AwaitFramePreviousRenderDone(FrameData& frame);
        void GPassContextWork(std::shared_ptr<RenderableGLTF> model, const glm::mat4& modelMatrix, std::vector<DrawContext>& drawContexts);
        void GPassCommandRecord(FrameData& frame);
        void LightSourceShadowWork(FrameData& frame);
        void SkyboxDraw(FrameData& frame);
        void LPassCommandRecord(FrameData& frame);
        void QueueGPUWork(VkCommandBuffer cmd, const std::vector<VkSemaphore>&& wait, VkSemaphore signal, VkFence fence = VK_NULL_HANDLE);

        void PreparePresentableImage(VkCommandBuffer cmd);

        void PrepareImguiFrame();
        void DrawImgui(VkCommandBuffer cmdBuffer, VkImage drawImage, VkImageView drawImageView, VkImageLayout srcLayout, VkImageLayout dstLayout);

        FrameData& GetCurrentFrame(){ return frames[frameNumber % FRAME_DATA_SIZE]; }

        //Testing
        std::unordered_map<std::string, std::shared_ptr<RenderableGLTF>> loadedScenes;
        Lights testLights;
        glm::vec4 light1Pos;
        glm::vec4 light2Pos;

        glm::mat4 modelMatrix;
        glm::mat4 modelMatrix2;
        glm::mat4 light1Matrix;
        glm::mat4 light2Matrix;

        void InitTestTextures();
        void InitTestData();
        void InitTestLightData();
        GPUMeshBuffers CreateRectangle();
    };
}
