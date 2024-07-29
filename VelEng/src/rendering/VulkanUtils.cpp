#include "VulkanUtils.h"

void Vel::TransitionImage(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout srcLayout, VkImageLayout dstLayout)
{
    VkImageAspectFlags aspectMask = (dstLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageMemoryBarrier2 imageBarrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,

        .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,

        .oldLayout = srcLayout,
        .newLayout = dstLayout,

        .image = image,
        .subresourceRange = {
            .aspectMask = aspectMask,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS
        }
    };

    VkDependencyInfo depInfo {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext = nullptr,

        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &imageBarrier
    };

    vkCmdPipelineBarrier2(cmdBuffer, &depInfo);
}

VkImageSubresourceRange Vel::CreateImageSubresourceRangeAll(VkImageAspectFlags aspect)
{
    VkImageSubresourceRange subresourceRange{
        .aspectMask = aspect,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS
    };

    return subresourceRange;
}

void Vel::ClearImage(VkCommandBuffer cmdBuffer, VkImage image, VkClearColorValue color, VkImageAspectFlagBits aspectFlags)
{
    VkClearColorValue clearColor = { {0.0f, 0.0f, 0.0f, 1.0f } };
    VkImageSubresourceRange clearRange = CreateImageSubresourceRangeAll(aspectFlags);
    vkCmdClearColorImage(cmdBuffer, image, VK_IMAGE_LAYOUT_GENERAL, &color, 1, &clearRange);
}

void Vel::BlitImage(VkCommandBuffer cmdBuffer, VkImage src, VkImage dst, VkExtent2D srcSize, VkExtent2D dstSize)
{
    VkImageBlit2 blitRegion{
        .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
        .pNext = nullptr,
        .srcSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .dstSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    blitRegion.srcOffsets[1].x = srcSize.width;
    blitRegion.srcOffsets[1].y = srcSize.height;
    blitRegion.srcOffsets[1].z = 1;

    blitRegion.dstOffsets[1].x = dstSize.width;
    blitRegion.dstOffsets[1].y = dstSize.height;
    blitRegion.dstOffsets[1].z = 1;

    VkBlitImageInfo2 blitInfo{
        .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
        .pNext = nullptr,
        .srcImage = src,
        .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .dstImage = dst,
        .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .regionCount = 1,
        .pRegions = &blitRegion,
        .filter = VK_FILTER_LINEAR
    };

    vkCmdBlitImage2(cmdBuffer, &blitInfo);
}
