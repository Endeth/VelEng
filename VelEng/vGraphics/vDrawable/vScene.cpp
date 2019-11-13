#include "VScene.h"

using namespace std;

namespace Vel
{
    Scene::Scene()
    {
        _sceneLighting = make_unique<SceneLighting>();
    }

    void Scene::DrawScene()
    {
        for (auto &Model : _models)
        {
            //Model->DrawModel();
        }
    }

    void Scene::DrawSceneWithImposedShader(const ShaderPtr& shader)
    {
        shader->Activate();

        for (auto &Model : _models)
        {
            //Model->SetModelMatrixUniform(shader);
            //Model->DrawModelWithImposedShader();
        }
        shader->Deactivate();
    }

    void Scene::DrawShadows()
    {
        _sceneLighting->DrawSceneShadows(_models);
    }

    void Scene::AddModel(const ModelPtr& model)
    {
        _models.push_back(model);
    }

    void Scene::AddLightSource(const LightPtr& lightSource)
    {
        _sceneLighting->AddLight(lightSource);
    }

    void Scene::CreateDirectionalLight(const glm::vec3 & direction, const LightSource::LightColor & color)
    {
        _sceneLighting->CreateDirectionalLight(direction, color);
    }

    void Scene::CreateDirectionalLight(unique_ptr<DirectionalLight>&& light)
    {
        _sceneLighting->CreateDirectionalLight(move(light));
    }
}