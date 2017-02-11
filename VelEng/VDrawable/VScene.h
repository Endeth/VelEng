#pragma once

#include "VModel.h"
#include "../VShaders/VGLSLShader.h"
#include "../VLights/VLight.h"
#include <memory>
#include <vector>

namespace Vel
{
	class VScene
	{
		using ShaderPtr = std::shared_ptr<VGLSLShader>;
		using ModelPtr = std::shared_ptr<VModel>;
		using LightPtr = std::shared_ptr<VLightSource>;

	public:
		VScene();
		void DrawScene();
		void DrawSceneWithImposedShader(const ShaderPtr& shader);
		void DrawShadows();
		void AddModel(const ModelPtr &model);
		void AddLightSource(const LightPtr &lightSource);
		void CreateDirectionalLight(const glm::vec3 &direction, const VLightSource::VLightColor &color);
		void CreateDirectionalLight(std::unique_ptr<VDirectionalLight> &&light);
		void SetLPassLightUniforms(GLuint lPassProgram) { _sceneLighting->SetLPassLightUniforms(lPassProgram); }
		void SetCameraPosition(const glm::vec3 &pos) { _sceneLighting->SetCameraPosition(pos); }
		void ActivateShadowMaps() { _sceneLighting->ActivateShadowMaps(); } //DEBUG

	private:
		std::vector<ModelPtr> _models; //TODO change to list maybe, add cleaning up
		std::unique_ptr<VSceneLighting> _sceneLighting;
	};
}