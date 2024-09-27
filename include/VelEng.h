#pragma once

#include "Actors/Actor.h"
#include "Rendering/Renderer.h"

struct SDL_Window;

namespace Vel
{
    class Engine
    {
    public:
        static Engine& Instance();

        void CreateScene();
        void Cleanup();
        void Run();

        Renderer* GetRenderer() { return &renderer; }

    private:
        Engine() = default;
        void Init();

        void HandleWindowEvents();

        bool isInitialized = false;
        bool stopRendering = false;
        bool quit = false;

        VkExtent2D windowExtent{ 1700 , 900 };

        SDL_Window* window = nullptr;
        SDL_Event sdlEvent;

        std::unique_ptr<Actor> scene;
        Renderer renderer;

        static Engine* instance;
    };
}