#include "Actors/Actor.h"

namespace Vel
{
    void Actor::AddChild(std::unique_ptr<Actor>&& other)
    {
        other->SetParent(this);
        children.emplace_back(std::move(other));
    }

    void Actor::AddComponent(std::unique_ptr<Component>&& other)
    {
        other->SetOwner(this);
        components.emplace_back(std::move(other));
    }

    glm::mat4 Actor::GetTransformationMatrix()
    {
        return glm::translate(GetWorldPosition());
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
