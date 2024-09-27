#include "Rendering/Frame/Frames.h"

void Vel::FrameData::Init(VkDevice device, uint32_t queueFamilyIndex)
{
    idx = 0;

    CreateCommandPools(device, queueFamilyIndex);
    CreateSynchronization(device);
    CreateResources();
    PrepareQueuesUnlockingOrder();
}

void Vel::FrameData::StartNew(uint32_t frameIdx)
{
    sync.workingOnCPU.store(true);
    ClearWorkQueues();

    for (auto& context : resources.gPassDrawContexts)
    {
        context.Clear();
    }
    resources.gPassDrawContexts.clear();

    resources.shadowDrawContext.Clear();

    frameEndCleanup.Flush();
    idx = frameIdx;
}

const uint64_t Vel::FrameData::GetFrameIdx() const
{
    return idx;
}

VkCommandPool Vel::FrameData::GetAvailableCommandPool()
{
    VkCommandPool pool;
    //Should always be able to return pool
    commandPools.TryPop(pool);

    return pool;
}

void Vel::FrameData::ReaddCommandPool(VkCommandPool pool)
{
    commandPools.Push(std::move(pool));
}

void Vel::FrameData::AddWork(const RenderQueueType queueType, std::function<void()>&& work, bool finalWork)
{
    for (auto& queue : frameWorkQueues)
    {
        if (queue.GetType() == queueType)
        {
            if (finalWork)
                queue.PushFinal(std::move(work));
            else
                queue.Push(std::move(work));

            if (!queue.IsLocked())
                onWorkAdd();

            return;
        }
    }
}

bool Vel::FrameData::HasWork()
{
    for (auto& queue : frameWorkQueues)
    {
        if (queue.HasWork())
            return true;
    }

    return false;
}

void Vel::FrameData::ClearWorkQueues()
{
    for (auto& queue : frameWorkQueues)
    {
        queue.Clear();
        queue.ClearOnFinishCallbacks();
    }
}

bool Vel::FrameData::GetWork(std::function<void()>& work)
{
    for (auto& queue : frameWorkQueues)
    {
        if (queue.HasWork())
            return queue.TryPop(work);
    }
}

void Vel::FrameData::AddQueueWorkFinishCallback(RenderQueueType queueType, std::function<void()>&& cb)
{
    for (auto& queue : frameWorkQueues)
    {
        if (queue.GetType() == queueType)
        {
            queue.AddOnFinishCallback(std::move(cb));

            return;
        }
    }
}

void Vel::FrameData::UnlockQueuesLock(const RenderStages lockType)
{
    bool hasWork = false;
    for (auto& queue : frameWorkQueues)
    {
        queue.Unlock(lockType);
        hasWork |= queue.HasWork();
    }

    //TODO check if unlocked queue has work
    if (hasWork)
        onQueueUnlock();
}

void Vel::FrameData::LockQueues()
{
    for (auto& queue : frameWorkQueues)
    {
        queue.LockAllLocks();
    }
}

void Vel::FrameData::Discard()
{
    for (auto& queue : frameWorkQueues)
    {
        queue.LockAllLocks();
    }

    ClearWorkQueues();
}

Vel::FrameData::Synchronization& Vel::FrameData::GetSync()
{
    return sync;
}

Vel::FrameData::SceneData& Vel::FrameData::GetSceneData()
{
    return sceneData;
}

void Vel::FrameData::Cleanup()
{
    sceneData.frameDescriptors.Cleanup();
    cleanupQueue.Flush();
}

void Vel::FrameData::CreateCommandPools(VkDevice device, uint32_t queueFamilyIndex)
{
    VkCommandPoolCreateInfo commandPoolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndex
    };

    for (int i = 0; i < RENDER_THREADS_COUNT + 1; ++i)
    {
        VkCommandPool pool;
        VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &pool));
        commandPools.Push(std::move(pool));
        frameEndCleanup.Push([p = pool, d = device, this]() { vkResetCommandPool(d, p, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT); });
        cleanupQueue.Push([p = pool, d = device, this]() { vkDestroyCommandPool(d, p, nullptr); });
    }
}

void Vel::FrameData::CreateSynchronization(VkDevice device)
{
    VkFenceCreateInfo fenceInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    VkSemaphoreCreateInfo workSemaphoreInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr
    };

    VkSemaphoreTypeCreateInfo timelineCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .pNext = NULL,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = 1
    };
    VkSemaphoreCreateInfo resourceSemaphoreInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &timelineCreateInfo
    };

    VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &sync.renderFence));
    cleanupQueue.Push([d = device, this]() { vkDestroyFence(d, sync.renderFence, nullptr); });

    VK_CHECK(vkCreateSemaphore(device, &workSemaphoreInfo, nullptr, &sync.swapchainSemaphore));
    VK_CHECK(vkCreateSemaphore(device, &workSemaphoreInfo, nullptr, &sync.skyboxWorkSemaphore));
    VK_CHECK(vkCreateSemaphore(device, &workSemaphoreInfo, nullptr, &sync.gPassWorkSemaphore));
    VK_CHECK(vkCreateSemaphore(device, &workSemaphoreInfo, nullptr, &sync.shadowsWorkSemaphore));
    VK_CHECK(vkCreateSemaphore(device, &workSemaphoreInfo, nullptr, &sync.lPassWorkSemaphore));

    cleanupQueue.Push([d = device, this]() {
        vkDestroySemaphore(d, sync.swapchainSemaphore, nullptr);
        vkDestroySemaphore(d, sync.skyboxWorkSemaphore, nullptr);
        vkDestroySemaphore(d, sync.gPassWorkSemaphore, nullptr);
        vkDestroySemaphore(d, sync.shadowsWorkSemaphore, nullptr);
        vkDestroySemaphore(d, sync.lPassWorkSemaphore, nullptr);
    });
}

void Vel::FrameData::CreateResources()
{
}

void Vel::FrameData::PrepareQueuesUnlockingOrder()
{
    std::vector<RenderStages> locks = { PREPARE_CPU_DATA };
    frameWorkQueues.emplace_back(GENERAL, locks); //This mostly simulates scene logic data

    //Important to properly divide scene logic to create proper queues/locks
    //Transfer scene data (camera and lights) occurs after world is finished updatind and GPU res not locked (render on the same data)
    //draw ima

    // CPU
    frameWorkQueues.emplace_back(MAIN_CONTEXT, locks, [this](){
        UnlockQueuesLock(PREPARE_MAIN_CONTEXT);
    });
    frameWorkQueues.emplace_back(SHADOW_CONTEXT, locks, [this]() {
        UnlockQueuesLock(PREPARE_SHADOW_CONTEXT);
    });

    std::vector<RenderStages> mainContextLocks = { PREPARE_MAIN_CONTEXT };
    frameWorkQueues.emplace_back(MAIN_COMMANDS_RECORD, mainContextLocks, [this]() {
        UnlockQueuesLock(RECORD_GPASS);
    });
    std::vector<RenderStages> shadowContextLocks = { PREPARE_SHADOW_CONTEXT };
    frameWorkQueues.emplace_back(SHADOW_COMMANDS_RECORD, shadowContextLocks, [this]() {
        UnlockQueuesLock(RECORD_SHADOWS);
    });
    std::vector<RenderStages> cpuWorkDoneLocks = { RECORD_GPASS, RECORD_SHADOWS };
    frameWorkQueues.emplace_back(CPU_WORK_DONE, cpuWorkDoneLocks, [this]() {
        UnlockQueuesLock(FRAME_PREPARED);
    });

    // GPU
    std::vector<RenderStages> queueLocks = { FRAME_PREPARED };
    frameWorkQueues.emplace_back(MAIN_COMMANDS_QUEUE, queueLocks, [this]() {
        UnlockQueuesLock(QUEUE_GPASS);
    });
    frameWorkQueues.emplace_back(SHADOW_COMMANDS_QUEUE, queueLocks, [this]() {
        UnlockQueuesLock(QUEUE_SHADOWS);
    });

    //TODO let's find out if adding lPass submition to shadows or gpass (depending on which one is last) queue submition is better
    std::vector<RenderStages> lPassLocks = { QUEUE_GPASS, QUEUE_SHADOWS /* PREPARE_CPU_DATA, RECORD_SHADOWS */};
    frameWorkQueues.emplace_back(LPASS_COMMANDS_RECORD, lPassLocks);
}
