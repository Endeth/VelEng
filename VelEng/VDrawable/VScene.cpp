
#include "VScene.h"

Vel::Scene::Scene()
{
	_sceneLighting = std::make_unique<SceneLighting>();
}

void Vel::Scene::DrawScene()
{
	for (auto &Model : _models)
	{
		Model->DrawModel();
	}
}

void Vel::Scene::DrawSceneWithImposedShader(const ShaderPtr& shader)
{
	shader->Activate();

	for (auto &Model : _models)
	{
		Model->SetModelMatrixUniform(shader);
		Model->DrawModelWithImposedShader();
	}
	shader->Deactivate();
}

void Vel::Scene::DrawShadows()
{
	_sceneLighting->DrawSceneShadows(_models);
}

void Vel::Scene::AddModel(const ModelPtr& model)
{
	_models.push_back(model);
}

void Vel::Scene::AddLightSource(const LightPtr& lightSource)
{
	_sceneLighting->AddLight(lightSource);
}

void Vel::Scene::CreateDirectionalLight(const glm::vec3 & direction, const LightSource::LightColor & color)
{
	_sceneLighting->CreateDirectionalLight(direction, color);
}

void Vel::Scene::CreateDirectionalLight(std::unique_ptr<DirectionalLight>&& light)
{
	_sceneLighting->CreateDirectionalLight(std::move(light));
}
