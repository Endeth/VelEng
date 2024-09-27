#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <algorithm>
#include <atomic>

namespace Vel
{
    template <typename Item>
    class TSQueue
    {
    public:
        //TODO resource guard returning resource to q in destructor maybe?
        //class ReturningGetter{};

        void Push(Item&& t)
        {
            std::lock_guard lock(mut);

            queue.push(std::forward<Item>(t));
        }

        bool TryPop(Item& item)
        {
            std::lock_guard lock(mut);

            if (queue.empty())
                return false;

            item = std::move(queue.front());
            queue.pop();

            return true;
        }

        size_t Size()
        {
            std::lock_guard lock(mut);
            return queue.size();
        }

        bool IsEmpty()
        {
            std::lock_guard lock(mut);
            return queue.empty();
        }

        void Clear()
        {
            std::lock_guard lock(mut);
            std::queue<Item> empty;
        }

    protected:
        std::mutex mut;
        std::queue<Item> queue;

    };

    template<typename Item, typename LockType>
    class LockedQueue : public TSQueue<Item>
    {
    public:
        LockedQueue(const std::vector<LockType>& lockTypes) :
            isLocked(true),
            locks()
        {
            for (auto lockType : lockTypes)
            {
                locks[lockType] = true;
            }
        }

        bool IsLocked()
        {
            return isLocked.load();
        }

        void Unlock(const LockType& type)
        {
            if (locks.contains(type))
                locks[type] = false;

            bool checkLocks = false;
            for (auto& lock : locks)
            {
                checkLocks |= lock.second;
            }

            isLocked.store(checkLocks);
        }

        void LockAllLocks()
        {
            for (auto& lock : locks)
            {
                lock.second = true;
            }
        }

    private:
        std::atomic<bool> isLocked;
        std::unordered_map<LockType, bool> locks;

    };

    class RenderWorkQueue : public LockedQueue<std::function<void()>, RenderStages>
    {
    public:
        RenderWorkQueue(const RenderQueueType queueType, const std::vector<RenderStages>& lockTypes) :
            LockedQueue(lockTypes), type(queueType)
        {
        }

        RenderWorkQueue(const RenderQueueType queueType, const std::vector<RenderStages>& lockTypes, std::function<void()>&& unlock) :
            LockedQueue(lockTypes), type(queueType), unlockFinishedCallback(unlock)
        {
        }

        void AddOnFinishCallback(std::function<void()>&& cb)
        {
            workFinishedCallbacks.emplace_back(std::move(cb));
        }

        void ClearOnFinishCallbacks()
        {
            workFinishedCallbacks.clear();
        }

        RenderQueueType GetType() const
        {
            return type;
        }

        bool HasWork()
        {
            return !IsEmpty() && !IsLocked();
        }

        void Push(std::function<void()>&& work)
        {
            std::lock_guard lock(mut);

            InternalPush(std::move(work));
        }

        void PushFinal(std::function<void()>&& finalItem)
        {
            std::lock_guard lock(mut);

            isFilled = true;

            InternalPush(std::move(finalItem));
        }

    private:
        void InternalPush(std::function<void()>&& item)
        {
            auto wrapper = [&, t = std::move(item)]() {
                t();

                if (isFilled && --workCount == 0 && (!workFinishedCallbacks.empty() || unlockFinishedCallback))
                {
                    if (unlockFinishedCallback)
                        unlockFinishedCallback();

                    for(auto& cb : workFinishedCallbacks)
                        cb();
                }
            };

            workCount++;
            queue.push(std::move(wrapper));
        }

        const RenderQueueType type;
        bool isFilled = false;
        std::vector<std::function<void()>> workFinishedCallbacks;
        std::function<void()> unlockFinishedCallback;
        std::atomic<int> workCount;
    };
}
