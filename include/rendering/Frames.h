#pragma once
#include "VulkanTypes.h"
#include "Descriptors.h"
#include "utils/DeletionQueue.h"
#include "utils/TSQueue.h"

namespace Vel
{
    class RenderThreadPool;

    class FrameData
    {
    public:
        friend RenderThreadPool;

        struct SceneData
        {
            AllocatedBuffer cameraDataBuffer;
            VkDescriptorSet cameraDescriptorSet;
            DescriptorAllocatorDynamic frameDescriptors;
        };

        struct Resources
        {
            //DrawContext context;
            AllocatedImage drawImage;

            //GPass framebuffer
            AllocatedImage position;
            AllocatedImage color;
            AllocatedImage normals;
            AllocatedImage metallicRoughness;
            AllocatedImage depth;
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

        void Init(VkDevice device, uint32_t queueFamilyIndex);
        //void Resize();

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
        uint32_t idx;

        TSQueue<VkCommandPool> commandPools;
        Synchronization sync;
        SceneData sceneData;

        std::list<RenderWorkQueue> frameWorkQueues;
        std::function<void()> onWorkAdd;
        std::function<void()> onQueueUnlock;

        DeletionQueue frameEndCleanup;
    };
}
