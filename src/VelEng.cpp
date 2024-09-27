#include "VelEng.h"
#include "Actors/RenderComponent.h"

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

    void Engine::HandleWindowEvents()
    {
        while (SDL_PollEvent(&sdlEvent) != 0)
        {
            if (sdlEvent.type == SDL_QUIT)
                quit = true;

            renderer.HandleSDLEvent(&sdlEvent);

            if (sdlEvent.type == SDL_WINDOWEVENT)
            {
                if (sdlEvent.window.event == SDL_WINDOWEVENT_MINIMIZED)
                {
                    stopRendering = true;
                }
                if (sdlEvent.window.event == SDL_WINDOWEVENT_RESTORED)
                {
                    stopRendering = false;
                }
            }
        }
    }

    void Engine::CreateScene()
    {
        scene = std::make_unique<Actor>();

        auto world = std::make_unique<Actor>();
        RenderComponent worldRenderer{ world.get()};

        auto light1 = std::make_unique<Actor>();
        auto light2 = std::make_unique<Actor>();
        RenderComponent lightRenderer{ world.get() };
        lightRenderer.SetRenderable();
        
        LightComponent lightComp1{ world.get() };
        LightComponent lightComp2{ world.get() };

        scene->AddChild(std::move(world));
        world->AddChild(std::move(light1));
        world->AddChild(std::move(light2));

    }

    void Engine::Cleanup()
    {
        if (isInitialized)
        {
            renderer.Cleanup();
            SDL_DestroyWindow(window);
        }
    }

    void Engine::Run()
    {
        while (!quit)
        {
            HandleWindowEvents();

            // do not draw if we are minimized
            if (stopRendering) {
                // TODO can we have a nice callback?
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            scene->OnFrameTick();

            renderer.Draw();
        }
    }
}
