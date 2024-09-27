#pragma once

#include <vector>
#include <memory>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include "Actors/Component.h"

namespace Vel
{
    class Component;

    class Actor
    {
    public:
        void AddChild(const Actor& other);
        void AddChild(Actor&& other);
        void AddChild(std::unique_ptr<Actor>&& other);
        void AddComponent(const Component& other);

        const glm::vec3& GetLocalPosition();
        glm::vec3 GetWorldPosition();
        void SetPosition(const glm::vec3& pos);
        void SetPositionInWorldSpace(const glm::vec3& pos);

        void SetParent(Actor* newParent);

        void OnFrameTick();

        std::vector<std::unique_ptr<Actor>>& GetChildren();

    private:
        Actor* parent;

        glm::vec3 position;

        std::vector<std::unique_ptr<Actor>> children;
        std::vector<std::unique_ptr<Component>> components;
    };
}