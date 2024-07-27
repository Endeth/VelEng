#pragma once
#include "VulkanTypes.h"
#include <SDL_events.h>

namespace Vel
{
    class Camera
    {
    public:
        void ProcessSDLEvent(SDL_Event& e);
        void Update();
        void SetPosition(glm::vec3 pos);

        glm::mat4 GetViewMatrix();
        glm::mat4 GetRotationMatrix();
        const glm::vec3& GetPosition()
        {
            return position;
        }

        float speed{ 0.25 };

    private:
        glm::vec3 position{ 0.f };
        glm::vec3 velocity{ 0.f };

        float pitch{ 0.f };
        float yaw{ 0.f };

        bool isRotating{ true };
    };
}
