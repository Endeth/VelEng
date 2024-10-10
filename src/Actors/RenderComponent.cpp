#include "Actors/RenderComponent.h"

#include "VelEng.h"

namespace Vel
{
    RenderComponent::RenderComponent(Actor* owner, Renderer* renderer) : Component(owner)
    {
        this->renderer = renderer;
    }
    void RenderComponent::SetRenderable(std::shared_ptr<IRenderable> renderable)
    {
        this->renderable = renderable;
    }
    void RenderComponent::OnFrameTick()
    {
        AddToDrawContext();
    }
    void RenderComponent::AddToDrawContext()
    {
        renderer->AddToRenderScene(renderable, owner->GetTransformationMatrix());
    }

    LightComponent::LightComponent(Actor* owner, Renderer* renderer) : Component(owner)
    {
        this->renderer = renderer;
    }

    void LightComponent::SetType(LightType lightType)
    {
        type = lightType;
    }

    void LightComponent::SetColor(const glm::vec3& col)
    {
        color = col;
    }

    void LightComponent::SetDirection(const glm::vec3& dir)
    {
        direction = dir;
    }

    void LightComponent::OnFrameTick()
    {
        AddToScene();
    }

    void LightComponent::AddToScene()
    {
        if (type == DIRECTIONAL)
        {
            renderer->AddSunlightToRenderScene(direction, color);
        }
        else if (type == POINT)
        {
            renderer->AddPointLightToRenderScene(owner->GetWorldPosition(), color);
        }
    }
}