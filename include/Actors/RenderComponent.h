#pragma once

#include "Actors/Component.h"
#include "Rendering/Scene/RenderTarget.h"

namespace Vel
{
    class RenderComponent : public Component
    {
    public:
        RenderComponent(Actor* owner);
        RenderComponent(const RenderComponent& other);
        void operator=(const RenderComponent&) = delete;
        RenderComponent(RenderComponent&&) = delete;
        void operator=(RenderComponent&&) = delete;

        void SetRenderable(IRenderable* renderable);

    protected:
        void OnFrameTick() override;

        void AddToDrawContext();

        Renderer* renderer;
        //RenderTarget renderTarget;
        // TODO get from asset manager
        IRenderable* renderable;

        //Mesh
        //Material
        //IsDirty
    };

    class LightComponent : Component
    {
    public:
        LightComponent(Actor* owner);
        LightComponent(const LightComponent& other);
        void operator=(const LightComponent&) = delete;
        LightComponent(LightComponent&&) = delete;
        void operator=(LightComponent&&) = delete;
    protected:
        void OnFrameTick() override;

        void AddToSceneLights();

        Renderer* renderer;

        //LightType type;
        glm::vec3 color;
        glm::vec3 direction;

        //IsDirty
    };
}