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

    VkImageSubresourceRange CreateImageSubresourceRangeAll(VkImageAspectFlags aspect);
    void TransitionImage(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout srcLayout, VkImageLayout dstLayout);
    void ClearImage(VkCommandBuffer cmdBuffer, VkImage image, VkClearColorValue color, VkImageAspectFlagBits aspectFlags);
    void BlitImage(VkCommandBuffer cmdBuffer, VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize);
}
