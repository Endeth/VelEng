#pragma once

#include <memory>
#include <vector>
#include "GL/glew.h"
#include "glm/glm.hpp"

#include "VFBOTextures.h"

namespace Vel
{
	class VFramebuffer
	{
		using TexturePtr = std::shared_ptr<VFramebufferTexture>;
		using DepthTexturePtr = std::shared_ptr<VDepthTexture>;
	public:
		VFramebuffer(const glm::ivec2& size);
		virtual ~VFramebuffer();

		void BindFBOWriting();
		void BindTexturesReading();
		void UnbindFBOWriting();
		void UnbindTexturesReading();

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

	class VGBufferFBO : public VFramebuffer
	{
	public:
		VGBufferFBO(const glm::ivec2& size);
		virtual ~VGBufferFBO();

	private:
		void AddColorAttachment(const std::shared_ptr<VFramebufferTexture>& texture);
		void AddDepthTextureAttachment();

	};

	class VShadowMap : public VFramebuffer
	{
	public:
		VShadowMap(const glm::ivec2& size);
		virtual ~VShadowMap();
	private:
		void AddDepthTextureAttachment();
	};
}