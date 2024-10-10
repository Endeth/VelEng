#pragma once

#include "Actors/Component.h"
#include "Rendering/Scene/Lighting.h"
#include "Rendering/Scene/RenderTarget.h"

namespace Vel
{
    // For now lets assume it can only hold GLTF
    class RenderComponent : public Component
    {
    public:
        RenderComponent(Actor* owner, Renderer* renderer);

        void SetRenderable(std::shared_ptr<IRenderable> renderable);

    protected:
        void OnFrameTick() override;

        void AddToDrawContext();

        // TODO remove renderer; maybe flip dependencies
        Renderer* renderer;
        // TODO get from asset manager
        std::shared_ptr<IRenderable> renderable;
    };

    class LightComponent : public Component
    {
    public:
        LightComponent(Actor* owner, Renderer* renderer);

        void SetType(LightType lightType);
        void SetColor(const glm::vec3& col);
        void SetDirection(const glm::vec3& dir);
    protected:
        void OnFrameTick() override;

        void AddToScene();

        // TODO remove renderer
        Renderer* renderer;

        // TODO point and directional can share 1 vector
        LightType type;
        glm::vec3 color;
        glm::vec3 direction;

        //IsDirty
    };
}