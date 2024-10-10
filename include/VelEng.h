#pragma once

#include "Actors/Actor.h"
#include "Rendering/Renderer.h"

struct SDL_Window;

namespace Vel
{
    class Engine
    {
    public:
        void Init();
        void LoadInitialScene();
        void Run();
        void Cleanup();

    private:

        void HandleWindowEvents();

        bool isInitialized = false;
        bool stopRendering = false;
        bool quit = false;

        VkExtent2D windowExtent{ 1700 , 900 };

        SDL_Window* window = nullptr;
        SDL_Event sdlEvent;

        std::unique_ptr<Actor> scene;
        Renderer renderer;

        std::shared_ptr<RenderableGLTF> testGLTFObject;
        std::shared_ptr<RenderableGLTF> testLightModel;

        static Engine* instance;
    };
}