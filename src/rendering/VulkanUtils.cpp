#include "Rendering/VulkanUtils.h"

VkImageMemoryBarrier2 Vel::GetImageMemoryBarrier(VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask, VkImageLayout srcLayout,
    VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask, VkImageLayout dstLayout)
{
    VkImageMemoryBarrier2 imageBarrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,

        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,

        .oldLayout = srcLayout,
        .newLayout = dstLayout,

        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS
        }
    };

    return imageBarrier;
}

void Vel::TransitionImage(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout srcLayout, VkImageLayout dstLayout)
{
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
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
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

void Vel::TransitionDepthImage(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout srcLayout, VkImageLayout dstLayout)
{
    VkImageMemoryBarrier2 imageBarrier{
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
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    VkDependencyInfo depInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext = nullptr,

        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &imageBarrier
    };

    vkCmdPipelineBarrier2(cmdBuffer, &depInfo);
}

void Vel::TransitionImages(VkCommandBuffer cmdBuffer, uint32_t count, VkImage* images, VkImageLayout* srcLayouts, VkImageLayout* dstLayouts)
{
    VkImageMemoryBarrier2* barriers = new VkImageMemoryBarrier2[count];
    for (uint32_t i = 0; i < count; ++i)
    {
        barriers[i] = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,

            .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,

            .oldLayout = srcLayouts[i],
            .newLayout = dstLayouts[i],

            .image = images[i],
            .subresourceRange = {
                .aspectMask = (VkImageAspectFlags)((dstLayouts[i] == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
                .baseMipLevel = 0,
                .levelCount = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS
            }
        };
    }

    VkDependencyInfo depInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext = nullptr,

        .imageMemoryBarrierCount = count,
        .pImageMemoryBarriers = barriers
    };

    vkCmdPipelineBarrier2(cmdBuffer, &depInfo);

    delete[] barriers;
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

void Vel::BlitImage(VkCommandBuffer cmdBuffer, VkImage src, VkImage dst, VkExtent3D srcSize, VkExtent3D dstSize)
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
    blitRegion.srcOffsets[1].z = srcSize.depth;

    blitRegion.dstOffsets[1].x = dstSize.width;
    blitRegion.dstOffsets[1].y = dstSize.height;
    blitRegion.dstOffsets[1].z = srcSize.depth;

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

void Vel::CopyDepthToColorImage(VkCommandBuffer cmdBuffer, VkImage src, AllocatableBuffer& buf, VkImage dst, VkExtent3D copySize)
{
    VkBufferImageCopy regions {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = 0,
        .imageExtent = copySize
    };

    /*//MOCKING RENDER TO IMAGE
    VkClearDepthStencilValue value = {};
    value.depth = 1.0f;
    value.stencil = 1;
    VkImageSubresourceRange range = {
        .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };
    TransitionDepthImage(cmdBuffer, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vkCmdClearDepthStencilImage(cmdBuffer, src, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &value, 1, &range);
    TransitionDepthImage(cmdBuffer, src, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL); //*/

    vkCmdCopyImageToBuffer(cmdBuffer, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buf.buffer, 1, &regions);

    regions.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    //std::vector<unsigned char> data(512 * 512 * 2, 255);
    //memcpy(buf.info.pMappedData, data.data(), 512 * 512 * 2);

    vkCmdCopyBufferToImage(cmdBuffer, buf.buffer, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &regions);
}

VkSemaphoreSubmitInfo Vel::CreateSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
{
    VkSemaphoreSubmitInfo info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = semaphore,
        //.value = 1,
        .stageMask = stageMask,
        .deviceIndex = 0
    };

    return info;
}

VkCommandBufferSubmitInfo Vel::CreateCommandBufferSubmitInfo(VkCommandBuffer cmdBuffer)
{
    VkCommandBufferSubmitInfo info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .pNext = nullptr,
        .commandBuffer = cmdBuffer,
        .deviceMask = 0
    };

    return info;
}

VkSubmitInfo2 Vel::CreateSubmitInfo(VkCommandBufferSubmitInfo& cmdBufferInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo, uint32_t waitSemaphoresCount,
    VkSemaphoreSubmitInfo* signalSemaphoreInfo, uint32_t signalSemaphoresCount)
{
    VkSubmitInfo2 info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .pNext = nullptr,

        .waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0U : waitSemaphoresCount,
        .pWaitSemaphoreInfos = waitSemaphoreInfo,

        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmdBufferInfo,

        .signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0U : signalSemaphoresCount,
        .pSignalSemaphoreInfos = signalSemaphoreInfo
    };

    return info;
}

VkRenderingAttachmentInfo Vel::CreateColorAttachmentInfo(VkImageView imageView)
{
    VkRenderingAttachmentInfo colorAttachment
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
    };

    return colorAttachment;
}
