#pragma once

#include "VModel.h"
#include "../../VShaders/VGLSLShader.h"
#include "../../VLights/VLight.h"
#include <memory>
#include <vector>

namespace Vel
{
	class VScene
	{
		using ShaderPtr = std::shared_ptr<VGLSLShader>;
	public:
		void DrawScene();
		void DrawSceneWithImposedShader(const ShaderPtr& shader);
		void AddModel(const std::shared_ptr<VModel> &model);
		void AddLightSource(const std::shared_ptr<VLightSource> &lightSource);

	private:
		std::vector<std::shared_ptr<VModel>> _models;
		std::vector<std::shared_ptr<VLightSource>> _lightSources;
	};
}