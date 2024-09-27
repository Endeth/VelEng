#include "Actors/RenderComponent.h"

#include "VelEng.h"

namespace Vel
{
    RenderComponent::RenderComponent(Actor* owner) : Component(owner)
    {
        auto& instance = Vel::Engine::Instance();
        renderer = instance.GetRenderer();
    }
    RenderComponent::RenderComponent(const RenderComponent& other)
    {
    }
    void RenderComponent::OnFrameTick()
    {
        AddToDrawContext();
    }
    void RenderComponent::AddToDrawContext()
    {
        renderer->AddToDrawContext(renderable.get());
    }
}