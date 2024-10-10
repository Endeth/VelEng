#include "Actors/Component.h"

namespace Vel
{
    Component::Component(Actor* owner) : owner(owner)
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