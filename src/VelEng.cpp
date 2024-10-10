#include "VelEng.h"
#include "Actors/RenderComponent.h"

namespace Vel
{
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

        LoadInitialScene();
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

    // TODO load from disk
    void Engine::LoadInitialScene()
    {
        scene = std::make_unique<Actor>();
        testGLTFObject = renderer.LoadGLTF(GET_MESH_PATH("sponza/Sponza.gltf"));
        testLightModel = renderer.LoadGLTF(GET_MESH_PATH("test/cube.glb"));

        auto world = std::make_unique<Actor>();
        auto worldRenderer = std::make_unique<RenderComponent>(world.get(), &renderer);
        worldRenderer->SetRenderable(testGLTFObject);
        auto globalLight = std::make_unique<LightComponent>(world.get(), &renderer);
        globalLight->SetType(DIRECTIONAL);
        globalLight->SetColor({ 0, 0, 255 });
        world->AddComponent(std::move(worldRenderer));
        world->AddComponent(std::move(globalLight));

        auto light1 = std::make_unique<Actor>();
        auto lightRenderer1 = std::make_unique<RenderComponent>(world.get(), &renderer);
        // TODO Create asset manager to populate asset pointers
        lightRenderer1->SetRenderable(testLightModel);
        auto lightComponent1 = std::make_unique<LightComponent>(world.get(), &renderer);
        lightComponent1->SetType(POINT);
        lightComponent1->SetColor({ 0, 0, 255 });
        light1->AddComponent(std::move(lightRenderer1));
        light1->AddComponent(std::move(lightComponent1));

        auto light2 = std::make_unique<Actor>();
        auto lightRenderer2 = std::make_unique<RenderComponent>(world.get(), &renderer);
        lightRenderer2->SetRenderable(testLightModel);
        auto lightComponent2 = std::make_unique<LightComponent>(world.get(), &renderer);
        lightComponent2->SetType(POINT);
        lightComponent2->SetColor({255, 0, 0});
        light2->AddComponent(std::move(lightRenderer2));
        light2->AddComponent(std::move(lightComponent2));

        world->AddChild(std::move(light1));
        world->AddChild(std::move(light2));
        scene->AddChild(std::move(world));
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
                // TODO can we have a nice callback? SDL_WINDOWEVENT_RESTORED maybe?
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            scene->OnFrameTick();

            renderer.Draw();
        }
    }
}
