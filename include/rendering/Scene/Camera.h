#pragma once
#include <SDL_events.h>

#include "Rendering/VulkanTypes.h"

namespace Vel
{
    class Camera
    {
    public:
        void ProcessSDLEvent(SDL_Event& e);
        void Update();
        void SetPosition(glm::vec3 pos);
        void SetProjection(float aspect, float fov);

        glm::mat4 GetViewMatrix() const;
        const glm::mat4& GetProjectionMatrix() const;
        glm::mat4 GetViewProjectionMatrix() const;
        glm::mat4 GetRotationMatrix() const;
        const glm::vec3& GetPosition() const
        {
            return position;
        }

        float speed{ 0.05f };

    private:
        glm::vec3 position{ 0.f };
        glm::vec3 velocity{ 0.f };
        glm::mat4 projection{ 1.f };

        float pitch{ 0.f };
        float yaw{ 0.f };

        bool isRotating{ true };
    };
}
