#include "rendering/Frames.h"

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
    ClearWorkQueues();
    frameEndCleanup.Flush();
    idx = frameIdx;
}

const uint32_t Vel::FrameData::GetFrameIdx() const
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

    VkSemaphoreCreateInfo semaphoreInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr
    };

    VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &sync.renderFence));
    cleanupQueue.Push([d = device, this]() { vkDestroyFence(d, sync.renderFence, nullptr); });

    VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &sync.swapchainSemaphore));
    VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &sync.skyboxSemaphore));
    VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &sync.gPassSemaphore));
    VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &sync.shadowsSemaphore));
    VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &sync.lPassSemaphore));
    cleanupQueue.Push([d = device, this]() {
        vkDestroySemaphore(d, sync.swapchainSemaphore, nullptr);
        vkDestroySemaphore(d, sync.skyboxSemaphore, nullptr);
        vkDestroySemaphore(d, sync.gPassSemaphore, nullptr);
        vkDestroySemaphore(d, sync.shadowsSemaphore, nullptr);
        vkDestroySemaphore(d, sync.lPassSemaphore, nullptr);
    });
}

void Vel::FrameData::CreateResources()
{
}

void Vel::FrameData::PrepareQueuesUnlockingOrder()
{
    std::vector<RenderStages> locks = { PREPARE_CPU_DATA };
    frameWorkQueues.emplace_back(GENERAL, locks);

    frameWorkQueues.emplace_back(MAIN_CONTEXT, locks, [this](){
        fmt::println("Unlock main context.                   Frame ID {}", idx);
        UnlockQueuesLock(PREPARE_MAIN_CONTEXT);
    });
    frameWorkQueues.emplace_back(SHADOW_CONTEXT, locks, [this]() {
        fmt::println("Unlock shadow context.                 Frame ID {}", idx);
        UnlockQueuesLock(PREPARE_SHADOW_CONTEXT);
    });

    locks = { PREPARE_MAIN_CONTEXT, PREPARE_SHADOW_CONTEXT };
    frameWorkQueues.emplace_back(PREPARED_FRAME_HANDLER, locks, [this]() {
        UnlockQueuesLock(FRAME_PREPARED);
    });

    locks = { FRAME_PREPARED };
    frameWorkQueues.emplace_back(MAIN_COMMANDS_RECORD, locks, [this]() {
        UnlockQueuesLock(RECORD_GPASS);
    });
    frameWorkQueues.emplace_back(SHADOW_COMMANDS_RECORD, locks, [this]() {
        UnlockQueuesLock(RECORD_SHADOWS);
    });

    locks = { RECORD_GPASS, RECORD_SHADOWS }; //TODO GPU semaphore might do the lpass stage trick
    frameWorkQueues.emplace_back(LPASS_COMMANDS_RECORD, locks);
}
