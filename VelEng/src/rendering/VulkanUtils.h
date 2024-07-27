#pragma once
#include "VulkanTypes.h"

#include <chrono>

namespace Vel
{
    class FunctionTimeMeasure
    {
    public:
        FunctionTimeMeasure(float& functionTime)
        {
            funcTime = &functionTime;
            start = std::chrono::system_clock::now();
        };
        ~FunctionTimeMeasure()
        {
            auto end = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            *funcTime = elapsed.count() / 1000.f;
        };

    private:
        std::chrono::system_clock::time_point start;
        float* funcTime;
    };

    void TransitionImage(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout srcLayout, VkImageLayout dstLayout);
}
