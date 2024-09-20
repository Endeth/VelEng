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

    class FrameData
    {
    public:
        friend RenderThreadPool;

        struct SceneData
        {
            AllocatableBuffer cameraDataBuffer;
            VkDescriptorSet cameraDescriptorSet;
            DescriptorAllocatorDynamic frameDescriptors;
        };

        struct Synchronization
        {
            VkSemaphore swapchainSemaphore;
            VkSemaphore skyboxSemaphore;
            VkSemaphore gPassSemaphore;
            VkSemaphore shadowsSemaphore; //TODO dynamic queue?
            VkSemaphore lPassSemaphore;
            VkFence renderFence;
            std::atomic<bool> inProgress = false;
            std::atomic<bool> preparing = false;
            std::atomic<bool> rendering = false;
        };

        //TODO maybe separate resources from work oriented code?
        struct FrameResources
        {
            std::vector<DrawContext> gPassDrawContexts;

            AllocatableImage lPassDrawImage;
            DeferredPasses::Framebuffer gPassFramebuffer;
        } resources;

        void Init(VkDevice device, uint32_t queueFamilyIndex);
        //void Resize(VkExtent3D size);

        void StartNew(uint32_t frameIdx);
        const uint32_t GetFrameIdx() const;

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
        uint32_t idx;

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
