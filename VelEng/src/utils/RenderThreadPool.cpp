#include "RenderThreadPool.h"
#include <iostream>

Vel::RenderThreadPool::RenderThreadPool(const size_t threadsCount) : stopPool(false)
{
    for (int i = 0; i < threadsCount; ++i)
    {
        workers.emplace_back(
            std::thread(&RenderThreadPool::ThreadWork, this)
        );
    }
}

void Vel::RenderThreadPool::Cleanup()
{
    stopPool.store(true);
    condVar.notify_all();

    for (auto& worker : workers)
        worker.join();
}

void Vel::RenderThreadPool::SetPrepareFrame(FrameData* frame)
{
    preparingFrame = frame;
}

void Vel::RenderThreadPool::SetWorkFrame(FrameData* frame)
{
    workFrame = frame;
}

void Vel::RenderThreadPool::SetupFrameData(FrameData& frame)
{
    frame.SetOnQueueUnlock([this]() {
        condVar.notify_all();
    });
    frame.SetOnWorkAdd([this]() {
        condVar.notify_one();
    });
}

void Vel::RenderThreadPool::ThreadWork()
{
    bool workFound = false;
    std::function<void()> work;

    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(workMutex);

            condVar.wait(lock, [&]()
            {
                workFound = GetWorkFromUnlockedQueue(work);
                return stopPool.load() || workFound;
            });

            if (stopPool.load())
                return;
        }

        if (workFound)
        {
            work();
            workFound = false;
        }
    }
}

bool Vel::RenderThreadPool::GetWorkFromUnlockedQueue(std::function<void()>& work)
{
    if (workFrame)
    {
        if (workFrame->HasWork())
            return workFrame->GetWork(work);
    }

    if (preparingFrame)
    {
        if (preparingFrame->HasWork())
            return preparingFrame->GetWork(work);
    }

    return false;
}
