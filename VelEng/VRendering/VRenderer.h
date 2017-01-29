#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "VFramebuffer.h"
#include "VGLSLShader.h"
#include "VScene.h"

namespace Vel
{
	class VRenderer
	{
	protected:
		using ScenePtr = std::shared_ptr<VScene>;
		ScenePtr _scene;
	public:
		VRenderer() {};
		virtual ~VRenderer() {};
		virtual void Render() = 0;
		void SetScene(const ScenePtr& scene) { _scene = scene; }

		//lights?
	};

	class VDefferedRenderer : public VRenderer
	{
		using ShaderPtr = std::shared_ptr<GLSLShader>;
	public:
		VDefferedRenderer(const glm::ivec2& resolution, const ShaderPtr &gPass, const ShaderPtr &lPass);
		void Render();
		void SetGPassShader(const ShaderPtr& shader){ _gPassShader = shader; }
		void SetLPassShader(const ShaderPtr& shader);
		void BindGBufferForWriting();
		void UnbindGBufferForWriting();
	private:
		void GeometryPass();
		void LightingPass();
		VGBufferFBO _gBuffer;
		ShaderPtr _gPassShader;
		ShaderPtr _lPassShader;

		class LightingPassQuad : public VBasicDrawableObject
		{
		public:
			LightingPassQuad();
			void SetVAO();
		private:
			void LoadIntoGPU();
			VArrayBuffer _vboVertices;
			VElementArrayBuffer _vboIndices;
		};
		LightingPassQuad _quad;
		

	};
};