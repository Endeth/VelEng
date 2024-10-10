#pragma once

#include <vector>
#include <memory>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include "Actors/Actor.h"

namespace Vel
{
    class Actor;

    class Component
    {
        friend class Actor;
    public:
        Component(Actor* owner);

        // TODO not a fan of changing component owner, let's make a component factory, so I won't have to use this on creation
        void SetOwner(Actor* newOwner);
        const Actor* GetOwner();

    protected:
        virtual void OnFrameTick() = 0;

        Actor* owner;
    };
}