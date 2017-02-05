#pragma once

#include <memory>
#include <vector>
#include "../VOpenGL/glew.h"
#include "../VOpenGL/glm/glm.hpp"

#include "../VTextures/VFBOTextures.h"

namespace Vel
{
	//flexible but requires manual work
	class VFramebuffer
	{
		using TexturePtr = std::shared_ptr<VFramebufferTexture>;
		using DepthTexturePtr = std::shared_ptr<VDepthTexture>;
	public:
		VFramebuffer(const glm::ivec2& size);
		virtual ~VFramebuffer();

		virtual void BindFBOWriting();
		virtual void BindTexturesReading();
		virtual void UnbindFBOWriting();
		virtual void UnbindTexturesReading();

		bool CheckStatus();

		virtual void AddColorAttachment(const TexturePtr& texture);
		virtual void AddDepthTextureAttachment();
		virtual void AddDepthRenderbufferAttachment();
	protected:
		GLuint _fboID{ 0 };
		int _texturesCount{ 0 };
		int _colorAttachments{ 0 };
		bool _attachedDepth{ false };
		bool _attachedStencil{ false };
		glm::ivec2 _size;

		std::vector<TexturePtr> _colorTextures;
		DepthTexturePtr _depthAttachment;
		GLuint _rboID;
		//StencilTexturePtr _stencilAttachment;
	};

	//construct and use for deferred rendering
	class VGBufferFBO : public VFramebuffer
	{
	public:
		VGBufferFBO(const glm::ivec2& size);
		virtual ~VGBufferFBO();
	};

	//construct and use for 2D only depth rendering
	class VShadowMap2D : public VFramebuffer
	{
	public:
		VShadowMap2D(const glm::ivec2& size = glm::ivec2(1024, 1024));
		virtual void BindTexturesReading() override;
		virtual void UnbindTexturesReading() override;
		void SetTextureUnit(GLuint textureUnit);
		virtual ~VShadowMap2D();
	private:
	};

	//construct and use for rendering depth cube
	class VShadowMapCube : public VFramebuffer
	{
	public:
		VShadowMapCube(const glm::ivec2& size = glm::ivec2(512, 512));
		virtual void BindTexturesReading() override;
		virtual void UnbindTexturesReading() override;
	protected:
		virtual void AddDepthTextureAttachment() override;
		std::shared_ptr<VDepthTextureCube> _depthAttachment;
	};
}