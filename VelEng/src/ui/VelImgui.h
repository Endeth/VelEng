#pragma once

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "ui/GUIValue.h"
#include <functional>

namespace Vel
{
    class Imgui
    {
    public:
        void Init(ImGui_ImplVulkan_InitInfo& initInfo, SDL_Window* window);
        void PrepareFrame(std::function<void(void)> windowPopulate);
        void HandleSDLEvent(SDL_Event* sdlEvent);
        void Draw(VkCommandBuffer cmd, VkImageView targetImageView, VkExtent2D& swapchainExtent);
        void Cleanup();
    };
}