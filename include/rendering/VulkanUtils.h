#pragma once

#include <chrono>

#include "Rendering/VulkanTypes.h"

#include "Rendering/Buffers/Buffers.h"


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
    void TransitionDepthImage(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout srcLayout, VkImageLayout dstLayout);
    void TransitionImages(VkCommandBuffer cmdBuffer, uint32_t count, VkImage* images, VkImageLayout* srcLayouts, VkImageLayout* dstLayouts);
    void ClearImage(VkCommandBuffer cmdBuffer, VkImage image, VkClearColorValue color, VkImageAspectFlagBits aspectFlags);
    void BlitImage(VkCommandBuffer cmdBuffer, VkImage src, VkImage dst, VkExtent3D srcSize, VkExtent3D dstSize);
    void CopyDepthToColorImage(VkCommandBuffer cmdBuffer, VkImage src, AllocatableBuffer& buf, VkImage dst, VkExtent3D copy);

    VkRenderingAttachmentInfo CreateColorAttachmentInfo(VkImageView imageView);
    //VkRenderingInfo CreateGeometryDrawRenderInfo(VkRenderingAttachmentInfo* color, uint32_t colorAttachmentsCount, VkRenderingAttachmentInfo* depth);
    VkSemaphoreSubmitInfo CreateSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
    VkCommandBufferSubmitInfo CreateCommandBufferSubmitInfo(VkCommandBuffer cmdBuffer);
    VkSubmitInfo2 CreateSubmitInfo(VkCommandBufferSubmitInfo& cmdBufferInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo, uint32_t waitSemaphoresCount,
        VkSemaphoreSubmitInfo* signalSemaphoreInfo, uint32_t signalSemaphoresCount);
}
