#pragma once

#include <deque>
#include <functional>

class DeletionQueue
{
public:
    void Push(std::function<void()>&& function)
    {
        deletors.push_back(std::move(function));
    }

    void Flush()
    {
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
        {
            (*it)();
        }

        deletors.clear();
    }

private:
    std::deque<std::function<void()>> deletors;
};