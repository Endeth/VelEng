
#include "VScene.h"

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

void Vel::VScene::AddModel(const std::shared_ptr<VModel>& model)
{
	_models.push_back(model);
}

void Vel::VScene::AddLightSource(const std::shared_ptr<VLightSource>& lightSource)
{
	_lightSources.push_back(lightSource);
}
