#pragma once

#include <memory>
#include <vector>

#include "vModel.h"
#include "vShaders/vGLSLShader.h"
#include "vLights/vLight.h"

namespace Vel
{
	class Scene
	{
		using ShaderPtr = std::shared_ptr<Shader>;
		using ModelPtr = std::shared_ptr<Model>;
		using LightPtr = std::shared_ptr<LightSource>;

	public:
		Scene();
		void DrawScene();
		void DrawSceneWithImposedShader(const ShaderPtr& shader);
		void DrawShadows();
		void AddModel(const ModelPtr &model);
		void AddLightSource(const LightPtr &lightSource);
		void CreateDirectionalLight(const glm::vec3 &direction, const LightSource::LightColor &color);
		void CreateDirectionalLight(std::unique_ptr<DirectionalLight> &&light);
		void SetLPassLightUniforms(uint32_t lPassProgram) { _sceneLighting->SetLPassLightUniforms(lPassProgram); }
		void SetCameraPosition(const glm::vec3 &pos) { _sceneLighting->SetCameraPosition(pos); }
		void ActivateShadowMaps() { _sceneLighting->ActivateShadowMaps(); } //DEBUG

	private:
		std::vector<ModelPtr> _models; //TODO change to batches
		std::unique_ptr<SceneLighting> _sceneLighting;
	};
}