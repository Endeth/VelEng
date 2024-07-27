#pragma once
#include "rendering/VulkanTypes.h"
#include "rendering/Renderer.h"

struct SDL_Window;

namespace Vel
{
    class Engine
    {
    public:
        static Engine& Instance();

        void Cleanup();
        void Draw();
        void Run();

    private:
        Engine() = default;
        void Init();

        bool isInitialized = false;
        bool stopRendering = false;

        VkExtent2D windowExtent{ 1700 , 900 };

        SDL_Window* window = nullptr;
        Renderer renderer;
        static Engine* instance;
    };
}