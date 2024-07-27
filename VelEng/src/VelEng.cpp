#include "VelEng.h"

namespace Vel
{
    Engine* Engine::instance = nullptr;

    Engine& Engine::Instance()
    {
        if (instance == nullptr)
        {
            instance = new Engine();
            instance->Init();
        }
        return *instance;
    }

    void Engine::Init()
    {
        fmt::println("VelEng Init");
        SDL_SetMainReady();
        SDL_Init(SDL_INIT_VIDEO);
        SDL_WindowFlags windowFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

        window = SDL_CreateWindow(
            "VelEng",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            windowExtent.width,
            windowExtent.height,
            windowFlags);

        renderer.Init(window, windowExtent);
        isInitialized = true;
    }
    void Engine::Cleanup()
    {
        if (isInitialized)
        {
            renderer.Cleanup();
            SDL_DestroyWindow(window);
        }
    }

    void Engine::Draw()
    {
        renderer.Draw();
    }

    void Engine::Run()
    {
        SDL_Event sdlEvent;
        bool quit = false;

        while (!quit)
        {
            while (SDL_PollEvent(&sdlEvent) != 0)
            {
                if (sdlEvent.type == SDL_QUIT)
                    quit = true;

                renderer.HandleSDLEvent(&sdlEvent);

                if (sdlEvent.type == SDL_WINDOWEVENT) {
                    if (sdlEvent.window.event == SDL_WINDOWEVENT_MINIMIZED) {
                        stopRendering = true;
                    }
                    if (sdlEvent.window.event == SDL_WINDOWEVENT_RESTORED) {
                        stopRendering = false;
                    }
                }
            }

            // do not draw if we are minimized
            if (stopRendering) {
                // throttle the speed to avoid the endless spinning
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            Draw();
        }
    }
}
