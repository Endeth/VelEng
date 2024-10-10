#include "Rendering/Frame/Swapchain.h"

#include "VkBootstrap.h"

void Vel::Swapchain::Init(VkPhysicalDevice physicalDevice, VkDevice dev, VkSurfaceKHR surface, VkExtent2D size)
{
    device = dev;

    builder = std::make_unique<vkb::SwapchainBuilder>(vkb::SwapchainBuilder{physicalDevice, device, surface});
    imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainExtent = size;
    presentMode = VK_PRESENT_MODE_FIFO_KHR;
    additionalUsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
    };

    CreateSwapchain();
}

VkImage Vel::Swapchain::GetImage(const FrameData& frame)
{
    return swapchainImages[frame.resources.swapchainImageIndex];
}

VkImageView Vel::Swapchain::GetImageView(const FrameData& frame)
{
    return swapchainImageViews[frame.resources.swapchainImageIndex];
}

void Vel::Swapchain::AcquireNextImageIndex(FrameData& frame)
{
    VkResult acquireResult = vkAcquireNextImageKHR(device, swapchain, FRAME_TIMEOUT, frame.GetSync().swapchainSemaphore, nullptr, &frame.resources.swapchainImageIndex);
    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        resizeRequested = true;
    }
}

void Vel::Swapchain::PresentImage(FrameData& frame, VkQueue presentQueue)
{
    presentInfo.pWaitSemaphores = &frame.GetSync().lPassWorkSemaphore;
    presentInfo.pImageIndices = &frame.resources.swapchainImageIndex;

    VkResult presentResult = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        resizeRequested = true;
    }
}

void Vel::Swapchain::CreateSwapchain()
{
    vkb::Swapchain vkbSwapchain = builder->set_desired_format(VkSurfaceFormatKHR{ .format = imageFormat, .colorSpace = colorSpace })
        .set_desired_present_mode(presentMode)
        .set_desired_extent(swapchainExtent.width, swapchainExtent.height)
        .add_image_usage_flags(additionalUsageFlags)
        .build()
        .value();

    swapchainExtent = vkbSwapchain.extent;
    swapchain = vkbSwapchain.swapchain;
    swapchainImages = vkbSwapchain.get_images().value();
    swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void Vel::Swapchain::Resize(SDL_Window* window)
{
    DestroySwapchain();

    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    CreateSwapchain();

    resizeRequested = false;
}

void Vel::Swapchain::DestroySwapchain()
{
    for (auto& swapchainImageView : swapchainImageViews)
    {
        vkDestroyImageView(device, swapchainImageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapchain, nullptr);
}
