#pragma once

#include "Rendering/VulkanTypes.h"

#include "Rendering/Frame/Frames.h"

namespace Vel
{
    class Swapchain
    {
    public:
        void Init(VkPhysicalDevice physicalDevice, VkDevice dev, VkSurfaceKHR surface, VkExtent2D size);
        void AcquireNextImageIndex(FrameData& frame);
        void PresentImage(FrameData& frame, VkQueue presentQueue);
        void DestroySwapchain();

        bool IsAwaitingResize() { return resizeRequested; };
        void Resize(SDL_Window* window);

        VkImage GetImage(const FrameData& frame);
        VkImageView GetImageView(const FrameData& frame);
        const VkFormat& GetImageFormat() const { return imageFormat; };
        VkExtent2D& GetImageSize() { return swapchainExtent; };
    private:
        VkDevice device;
        std::unique_ptr<vkb::SwapchainBuilder> builder;
        VkSwapchainKHR swapchain;
        VkFormat imageFormat;
        VkColorSpaceKHR colorSpace;
        VkPresentModeKHR presentMode;
        VkImageUsageFlagBits additionalUsageFlags;

        VkPresentInfoKHR presentInfo; //TODO make thread safe

        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImageViews;
        //uint32_t imageIndex;
        VkExtent2D swapchainExtent;
        bool resizeRequested = false;

        void CreateSwapchain();
    };
}
