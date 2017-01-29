#pragma once

#include "VModel.h"
#include "VGLSLShader.h"
#include "VLight.h"
#include <memory>
#include <vector>

namespace Vel
{
	class VScene
	{
		using ShaderPtr = std::shared_ptr<GLSLShader>;
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