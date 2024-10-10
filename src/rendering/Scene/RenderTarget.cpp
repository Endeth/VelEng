#include "Rendering/Scene/RenderTarget.h"

void Vel::RenderTarget::InitScene(std::list<IRenderable*> loadedModels)
{
    for (const auto renderable : loadedModels)
    {
        sceneObjects[renderable] = std::vector<glm::mat4>();
    }
}

void Vel::RenderTarget::AddModelInstance(IRenderable* model, const glm::mat4& matrix)
{
    sceneObjects[model].push_back(matrix);
}

void Vel::RenderTarget::SetSunlight(const glm::vec3& direction, const glm::vec3& color)
{
    lights.sunlight.SetLightData(direction, color);
}

void Vel::RenderTarget::AddPointLight(const glm::vec3& position, const glm::vec3& color)
{
    lights.pointLights.push_back({ .position = glm::vec4(position, 1.0f), .color = glm::vec4(color, 1.0f) });
}

Vel::SceneLights& Vel::RenderTarget::GetSceneLights()
{
    return lights;
}

uint32_t Vel::RenderTarget::GetPointLightsCount() const
{
    return lights.pointLights.size();
}

void Vel::RenderTarget::FillContext(DrawContext& ctx)
{
    for (const auto& obj : sceneObjects)
    {
        for (const auto& mat : obj.second)
        {
            obj.first->Draw(mat, ctx);
        }
    }
}

void Vel::RenderTarget::Reset()
{
    // TODO instead of lets just update
    sceneObjects.clear();
    lights.pointLights.clear();
}
