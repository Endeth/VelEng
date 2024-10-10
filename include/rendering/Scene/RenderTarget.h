#pragma once

#include <memory>

#include "Rendering/Scene/Lighting.h"
#include "Rendering/Scene/Renderable.h"

namespace Vel
{
    class RenderTarget
    {
    public:
        void InitScene(std::list<IRenderable*> loadedModels);

        void AddModelInstance(IRenderable* model, const glm::mat4& matrix);

        void SetSunlight(const glm::vec3& direction, const glm::vec3& color);
        void AddPointLight(const glm::vec3& position, const glm::vec3& color);
        SceneLights& GetSceneLights();
        uint32_t GetPointLightsCount() const;

        void FillContext(DrawContext& ctx);

        void Reset();

    private:
        using Instances = std::vector<glm::mat4>;
        SceneLights lights;
        std::unordered_map<IRenderable*, Instances> sceneObjects;
    };
}
