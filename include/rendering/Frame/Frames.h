#pragma once

#include "Rendering/VulkanTypes.h"

#include "Rendering/Buffers/Buffers.h"

#include "Rendering/RenderPasses/Descriptors.h"
#include "Rendering/RenderPasses/DeferredPasses.h"

#include "Utils/DeletionQueue.h"
#include "Utils/TSQueue.h"

namespace Vel
{
    class RenderThreadPool;

    // TODO divide data into cpu and gpu
    class FrameData
    {
    public:
        friend RenderThreadPool;

        struct SceneData
        {
            DescriptorAllocatorDynamic frameDescriptors;

            AllocatableBuffer cameraDataBuffer;
            VkDescriptorSet cameraDescriptorSet;

            // Containts all light information (including info about point light buffers)
            AllocatableBuffer globalLightsDataBuffer;
            VkDescriptorSet globalLightsDescriptorSet;
            AllocatableBuffer pointLightsDataBuffer;
        };

        struct Synchronization
        {
            VkSemaphore swapchainSemaphore;
            VkSemaphore skyboxWorkSemaphore;
            VkSemaphore gPassWorkSemaphore;
            VkSemaphore shadowsWorkSemaphore; //TODO dynamic queue?
            VkSemaphore lPassWorkSemaphore;

            std::condition_variable CPUCondVar;
            std::atomic<bool> workingOnCPU = false;
            std::condition_variable GPUCondVar;
            std::atomic<bool> workingOnGPU = false;
            std::mutex workMutex;

            VkFence renderFence;
        };

        //TODO maybe separate resources from work oriented code?
        struct FrameResources
        {
            //Camera CPU data buffer
            //Lights CPU data buffer

            std::vector<DrawContext> gPassDrawContexts;
            DrawContext shadowDrawContext;

            VkCommandBuffer gPassDrawCommand;
            VkCommandBuffer shadowsDrawCommand;

            AllocatableImage lPassDrawImage;
            DeferredPasses::Framebuffer gPassFramebuffer;

            uint32_t swapchainImageIndex;
            //Maybe all shadowmaps
        } resources;

        void Init(VkDevice device, uint32_t queueFamilyIndex);
        //void Resize(VkExtent3D size);

        void StartNew(uint32_t frameIdx);
        const uint64_t GetFrameIdx() const;

        VkCommandPool GetAvailableCommandPool();
        void ReaddCommandPool(VkCommandPool pool);

        void AddWork(const RenderQueueType queueType, std::function<void()>&& work, bool finalWork = false);
        bool GetWork(std::function<void()>& work);
        bool HasWork();
        void ClearWorkQueues();

        void AddQueueWorkFinishCallback(RenderQueueType queueType, std::function<void()>&& cb);
        void UnlockQueuesLock(const RenderStages lockType);
        void LockQueues();
        void Discard();

        Synchronization& GetSync();
        SceneData& GetSceneData();

        void Cleanup();
        DeletionQueue cleanupQueue;
    private:
        uint64_t idx;

        TSQueue<VkCommandPool> commandPools;
        Synchronization sync;
        SceneData sceneData;

        std::list<RenderWorkQueue> frameWorkQueues;
        std::function<void()> onWorkAdd;
        std::function<void()> onQueueUnlock;

        DeletionQueue frameEndCleanup;

        void CreateCommandPools(VkDevice device, uint32_t queueFamilyIndex);
        void CreateSynchronization(VkDevice device);
        void CreateResources();

        void SetOnWorkAdd(std::function<void()>&& cb)
        {
            onWorkAdd = std::move(cb);
        }
        void SetOnQueueUnlock(std::function<void()>&& cb)
        {
            onQueueUnlock = std::move(cb);
        }

        void PrepareQueuesUnlockingOrder();
    };
}
