#pragma once

#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>

#include "rendering/VulkanTypes.h"
#include "rendering/Frame/Frames.h"
#include "utils/TSQueue.h"

namespace Vel
{
    class RenderThreadPool
    {
    public:
        RenderThreadPool();

        void Init(const size_t threadsCount);
        void Cleanup();
        void SetPrepareFrame(FrameData* frame);
        void SetWorkFrame(FrameData* frame);
        void SetupFrameData(FrameData& frame);

    protected:
        void ThreadWork();
        bool GetWorkFromUnlockedQueue(std::function<void()>& work);

        std::atomic<bool> stopPool;
        std::vector<std::thread> workers;
        std::mutex workMutex;
        std::condition_variable condVar;

        FrameData* workFrame;
        FrameData* preparingFrame;
    };
}
