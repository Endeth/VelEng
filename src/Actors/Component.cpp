#include "Actors/Component.h"

namespace Vel
{
    Component::Component(Actor* owner) : owner(owner)
    {
    }

    Component::Component(const Component& other)
    {
    }

    void Component::SetOwner(Actor* newOwner)
    {
        owner = newOwner;
    }

    const Actor* Component::GetOwner()
    {
        return owner;
    }
}