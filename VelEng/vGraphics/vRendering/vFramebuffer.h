#pragma once

#include <memory>
#include <vector>

#include "external/glm/glm.hpp"
#include "vGraphics/vTextures/vFBOTextures.h"

namespace Vel
{
	//flexible but requires manual work
	class OpenGLFramebuffer
	{
		using TexturePtr = std::shared_ptr<FramebufferTexture>;
		using DepthTexturePtr = std::shared_ptr<DepthTexture>;
	public:
		OpenGLFramebuffer(const glm::ivec2& size);
		virtual ~OpenGLFramebuffer();

		virtual void BindFBOWriting();
		virtual void BindTexturesReading();
		virtual void UnbindFBOWriting();
		virtual void UnbindTexturesReading();

		bool CheckStatus();

		virtual void AddColorAttachment(const TexturePtr& texture);
		virtual void AddDepthTextureAttachment();
		virtual void AddDepthRenderbufferAttachment();
	protected:
		uint32_t _fboID{ 0 };
		int _texturesCount{ 0 };
		int _colorAttachments{ 0 };
		bool _attachedDepth{ false };
		bool _attachedStencil{ false };
		glm::ivec2 _size;

		std::vector<TexturePtr> _colorTextures;
		DepthTexturePtr _depthAttachment;
		uint32_t _rboID;
		//StencilTexturePtr _stencilAttachment;
	};

	//construct and use for deferred rendering
	class GBufferFBO : public OpenGLFramebuffer
	{
	public:
		GBufferFBO(const glm::ivec2& size);
		virtual ~GBufferFBO();
	};

	//construct and use for 2D only depth rendering
	class ShadowMap2D : public OpenGLFramebuffer
	{
	public:
		ShadowMap2D(const glm::ivec2& size = glm::ivec2(1024, 1024));
		virtual void BindTexturesReading() override;
		virtual void UnbindTexturesReading() override;
		void SetTextureUnit(uint32_t textureUnit);
		virtual ~ShadowMap2D();
	private:
	};

	//construct and use for rendering depth cube
	class ShadowMapCube : public OpenGLFramebuffer
	{
	public:
		ShadowMapCube(const glm::ivec2& size = glm::ivec2(512, 512));
		virtual void BindTexturesReading() override;
		virtual void UnbindTexturesReading() override;
		void SetTextureUnit(uint32_t textureUnit);
	protected:
		virtual void AddDepthTextureAttachment() override;
		std::shared_ptr<DepthTextureCube> _depthAttachment;
	};
}