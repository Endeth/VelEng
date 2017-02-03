
#include "VScene.h"

Vel::VScene::VScene()
{
	_sceneLighting = std::make_unique<VSceneLighting>();
}

void Vel::VScene::DrawScene()
{
	for (auto &Model : _models)
	{
		Model->DrawModel();
	}
}

void Vel::VScene::DrawSceneWithImposedShader(const ShaderPtr& shader)
{
	shader->Activate();

	for (auto &Model : _models)
	{
		Model->SetModelMatrixUniform(shader);
		Model->DrawModelWithImposedShader();
	}
	shader->Deactivate();
}

void Vel::VScene::DrawShadows()
{
	_sceneLighting->DrawSceneShadows(_models);
}

void Vel::VScene::AddModel(const ModelPtr& model)
{
	_models.push_back(model);
}

void Vel::VScene::AddLightSource(const LightPtr& lightSource)
{
	_sceneLighting->AddLight(lightSource);
}
