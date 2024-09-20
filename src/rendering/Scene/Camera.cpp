#include "Rendering/Scene/Camera.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

void Vel::Camera::ProcessSDLEvent(SDL_Event& e)
{
    if (e.type == SDL_KEYDOWN)
    {
        if (e.key.keysym.sym == SDLK_w)
        {
            velocity.z = -1;
        }
        if (e.key.keysym.sym == SDLK_s)
        {
            velocity.z = 1;
        }
        if (e.key.keysym.sym == SDLK_a)
        {
            velocity.x = -1;
        }
        if (e.key.keysym.sym == SDLK_d)
        {
            velocity.x = 1;
        }
        if (e.key.keysym.sym == SDLK_SPACE)
        {
            velocity.y = 1;
        }
        if (e.key.keysym.sym == SDLK_LCTRL)
        {
            velocity.y = -1;
        }
        if (e.key.keysym.sym == SDLK_F1)
        {
            isRotating = !isRotating;
        }
    }

    if (e.type == SDL_KEYUP)
    {
        if (e.key.keysym.sym == SDLK_w)
        {
            velocity.z = 0;
        }
        if (e.key.keysym.sym == SDLK_s)
        {
            velocity.z = 0;
        }
        if (e.key.keysym.sym == SDLK_a)
        {
            velocity.x = 0;
        }
        if (e.key.keysym.sym == SDLK_d)
        {
            velocity.x = 0;
        }
        if (e.key.keysym.sym == SDLK_SPACE)
        {
            velocity.y = 0;
        }
        if (e.key.keysym.sym == SDLK_LCTRL)
        {
            velocity.y = 0;
        }
    }

    if (e.type == SDL_MOUSEMOTION && isRotating)
    {
        yaw += (float)e.motion.xrel / 200.f;
        pitch -= (float)e.motion.yrel / 200.f;
    }
}

void Vel::Camera::Update()
{
    glm::mat4 cameraRotation = GetRotationMatrix();
    position += glm::vec3(cameraRotation * glm::vec4(velocity * speed, 0.f));
}

glm::mat4 Vel::Camera::GetViewMatrix() const
{
    glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.f), position);
    glm::mat4 cameraRotation = GetRotationMatrix();
    return glm::inverse(cameraTranslation * cameraRotation);
}

const glm::mat4& Vel::Camera::GetProjectionMatrix() const
{
    return projection;
}

glm::mat4 Vel::Camera::GetViewProjectionMatrix() const
{
    return projection * GetViewMatrix();
}

glm::mat4 Vel::Camera::GetRotationMatrix() const
{
    glm::quat pitchRotation = glm::angleAxis(pitch, glm::vec3{ 1.f, 0.f, 0.f });
    glm::quat yawRotation = glm::angleAxis(yaw, glm::vec3{ 0.f, -1.f, 0.f });

    return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
}

void Vel::Camera::SetPosition(glm::vec3 pos)
{
    position = pos;
}

void Vel::Camera::SetProjection(float aspect, float fov)
{
    projection = glm::perspective(glm::radians(fov), aspect, 10000.f, 0.01f);
    projection[1][1] *= -1;
}
