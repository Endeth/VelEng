#pragma once
#include "VulkanTypes.h"
#include "utils/DeletionQueue.h"

#include "Utils/RenderThreadPool.h"
#include "Rendering/Assets/GLTFObjectLoader.h"

#include "Rendering/Buffers/Buffers.h"
#include "Rendering/Buffers/GPUAllocator.h"

#include "Rendering/Frame/Frames.h"
#include "Rendering/Frame/Swapchain.h"

#include "Rendering/RenderPasses/DeferredPasses.h"
#include "Rendering/RenderPasses/GLTFMaterialPass.h"
#include "Rendering/RenderPasses/Pipeline.h"
#include "Rendering/RenderPasses/ShadowPass.h"
#include "Rendering/RenderPasses/SkyboxPass.h"

#include "Rendering/Scene/Camera.h"
#include "Rendering/Scene/Lighting.h"
#include "Rendering/Scene/Renderable.h"
#include "Rendering/Scene/RenderTarget.h"

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

    //TODO get this to scene area
    struct Lights
    {
        glm::vec4 ambient;
        Sunlight sunlight;
        PointLight pointLights[2]; //TODO make dynamic or big buffer?
    };

    class RenderTarget;

    class Renderer
    {
    public:
        Renderer(); //Only for RenderThread
        ~Renderer();

        //Configure() before Init?

        void Init(SDL_Window* sdlWindow, const VkExtent2D& windowExtent);

        void HandleSDLEvent(SDL_Event* sdlEvent);
        void Draw();
        void AddToDrawContext(IRenderable* target);
        void SetDrawContextAsFilled();

        GPUAllocator* GetAllocator(){ return &gpuAllocator; }
        const DeferredPasses& GetDeferredRenderer() const { return deferredPasses; }

        AllocatableImage whiteImage;
        AllocatableImage defaultNormalsImage;
        AllocatableImage defaultMetallicRoughnessImage;
        AllocatableImage defaultCubeImage;
        AllocatableImage errorCheckerboardImage;
        VkSampler defaultSamplerLinear;
        VkSampler shadowSamplerNearest;

        void Cleanup();

    private:
        SDL_Window* window;
        bool isInitialized = false;
        RenderThreadPool renderThreadPool;


        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkSurfaceKHR surface;
        // TODO make a stack/queue of available queues for threads
        VkQueue graphicsQueue;
        // TODO gather more families and create submits based on available families
        // map<OperationType, TSQueue<VkQueue>>
        // but I don't think it will make that much of a difference
        uint32_t graphicsQueueFamily;
        VkCommandBufferBeginInfo primaryCommandBegin;

        Swapchain swapchain;

        GPUAllocator gpuAllocator;

        //Frame specific data
        uint32_t frameNumber = 0;
        constexpr static size_t FRAME_DATA_SIZE = 2;
        FrameData frames[FRAME_DATA_SIZE];

        GLTFObjectLoader meshLoader;

        SkyboxPass skyboxPass;
        ShadowPass shadowPass;
        DeferredPasses deferredPasses;
        std::mutex drawContextMutex;
        std::mutex graphicsQueueMutex;
        VkExtent3D drawExtent;
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

        //TODO kinda same
        void CreateFrameData();
        void CreateFramesGPUData();

        void CreateCommandsInfo();
        VkCommandBuffer CreateCommandBuffer(VkCommandPool pool, VkCommandBufferLevel level);
        void CreateAllocator();

        void InitSkyboxPass();
        void InitShadowPass();
        void InitDeferred();
        void InitImgui();

        void UpdateScene();
        void UpdateWorldActors();
        void UpdateCameraCPUBuffer();
        void UpdateCameraDescriptorsData(FrameData& frame);
        void UpdateGlobalLighting();
        void UpdateLightDescriptorsData(FrameData& frame);

        void AwaitFrameRenderDone(FrameData& frame);
        void AwaitTimelineSemaphore(VkSemaphore* semaphore);

        // CPU
        void GPassContextWork(std::shared_ptr<RenderableGLTF> model, const glm::mat4& modelMatrix, std::vector<DrawContext>& drawContexts);
        void GPassCommandsRecord(FrameData& frame);
        void ShadowMapContextWork(FrameData& frame);
        void ShadowMapCommandsRecord(FrameData& frame);

        // GPU mainly
        void SkyboxDraw(FrameData& frame);
        void GPassQueueSubmit(FrameData& frame);
        void ShadowMapQueueSubmit(FrameData& frame);
        void LPassCommandRecord(FrameData& frame);

        void PreparePresentableImage(VkCommandBuffer cmd, FrameData& frame);

        void PrepareImguiFrame();
        void DrawImgui(VkCommandBuffer cmdBuffer, VkImage drawImage, VkImageView drawImageView, VkImageLayout srcLayout, VkImageLayout dstLayout);

        FrameData& GetCurrentFrame(){ return frames[frameNumber % FRAME_DATA_SIZE]; }
        FrameData& GetFillableFrame(){ return frames[frameNumber % FRAME_DATA_SIZE]; }

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
