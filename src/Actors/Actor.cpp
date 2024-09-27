#include "Actors/Actor.h"

namespace Vel
{
    void Actor::AddChild(const Actor& other)
    {
        auto& newChild = children.emplace_back(other);
        newChild->SetParent(this);
    }

    void Actor::AddChild(Actor&& other)
    {
        // TODO set pointer, delete from other's parent
    }

    void Actor::AddChild(std::unique_ptr<Actor>&& other)
    {
        components.emplace_back(std::move(other));
    }

    void Actor::AddComponent(const Component& other)
    {
        auto& newComponent = components.emplace_back(other);
        newComponent->SetOwner(this);
    }

    const glm::vec3& Actor::GetLocalPosition()
    {
        return position;
    }

    glm::vec3 Actor::GetWorldPosition()
    {
        glm::vec3 worldPosition = position;
        if (parent)
            worldPosition += parent->GetWorldPosition();

        return worldPosition;
    }

    void Actor::SetPosition(const glm::vec3& pos)
    {
        position = pos;
    }

    void Actor::SetPositionInWorldSpace(const glm::vec3& pos)
    {
        glm::vec3 parentPosition;
        if (parent)
            parentPosition = GetWorldPosition();
        else
            parentPosition = { 0, 0, 0 };

        position = pos - parentPosition;
    }

    void Actor::SetParent(Actor* newParent)
    {
        parent = newParent;
    }

    void Actor::OnFrameTick()
    {
        for (auto& component : components)
            component->OnFrameTick();

        for (auto& child : children)
            child->OnFrameTick();
    }

    std::vector<std::unique_ptr<Actor>>& Actor::GetChildren()
    {
        return children;
    }
}
