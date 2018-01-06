#pragma once

#include "external/glm/glm.hpp"
#include "vRendering/vFramebuffer.h"
#include "vShaders/vGLSLShader.h"
#include "vDrawable/vScene.h"
#include "vLights/vLight.h"

namespace Vel
{
	class Renderer
	{
	protected:
		using ScenePtr = std::shared_ptr<Scene>;
		ScenePtr _scene;
	public:
		Renderer() {};
		virtual ~Renderer() {};
		virtual void Render() = 0;
		void SetScene(const ScenePtr& scene) { _scene = scene; }

		//lights?
	};

	class DefferedRenderer : public Renderer
	{
		using ShaderPtr = std::shared_ptr<Shader>;
	public:
		DefferedRenderer(const glm::ivec2& resolution, const ShaderPtr &gPass, const ShaderPtr &lPass);
		virtual void Render() override;
		void SetGPassShader(const ShaderPtr& shader){ _gPassShader = shader; }
		void SetLPassShader(const ShaderPtr& shader);
		void BindGBufferForWriting();
		void BindShadowMapReading() { _lPassShader->Activate(); _scene->ActivateShadowMaps(); } //DEBUG
		void UnbindGBufferForWriting();
	private:
		void GeometryPass();
		void LightingPass();
		GBufferFBO _gBuffer;
		ShaderPtr _gPassShader;
		ShaderPtr _lPassShader;

		class LightingPassQuad : public BasicDrawableObject
		{
		public:
			LightingPassQuad();
			void SetVAO();
		private:
			void LoadIntoGPU();
			ArrayBuffer _vboVertices;
			ElementArrayBuffer _vboIndices;
		};
		LightingPassQuad _quad;
		

	};
};