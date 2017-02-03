
#include "VFramebuffer.h"

namespace Vel
{
	VFramebuffer::VFramebuffer(const glm::ivec2& size) : _size(size)
	{
		glGenFramebuffers(1, &_fboID);
	}

	VFramebuffer::~VFramebuffer()
	{
		glDeleteFramebuffers(1, &_fboID);
	}

	void VFramebuffer::BindFBOWriting()
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fboID);
	}

	void VFramebuffer::BindTexturesReading()
	{
		for (auto &tex : _colorTextures)
		{
			tex->ActivateTextureUnit();
			tex->BindTexture(); 
		}
		if (_depthAttachment != nullptr)
		{
			_depthAttachment->ActivateTextureUnit();
			_depthAttachment->BindTexture();
		}
	}

	void VFramebuffer::UnbindFBOWriting()
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	void VFramebuffer::UnbindTexturesReading()
	{
		for (auto &tex : _colorTextures)
		{
			tex->ActivateTextureUnit();
			tex->UnbindTexture();
		}
		if (_depthAttachment != nullptr)
		{
			_depthAttachment->ActivateTextureUnit();
			_depthAttachment->UnbindTexture();
		}
	}

	bool VFramebuffer::CheckStatus()
	{
		auto Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE) 
		{
			printf("FB error, status: 0x%x\n", Status);
			return false;
		}
		return true;
	}

	void VFramebuffer::AddColorAttachment(const TexturePtr & texture)
	{
		_colorTextures.push_back(texture);
		texture->SetTextureUnit(GL_TEXTURE0 + _texturesCount);
		texture->AttachToFBO(GL_COLOR_ATTACHMENT0 + _colorAttachments);
		_colorAttachments++;
		_texturesCount++;
	}

	//fixed precision
	void VFramebuffer::AddDepthTextureAttachment()
	{
		_depthAttachment = std::make_shared<VDepthTexture>(_size);
		_depthAttachment->SetTextureUnit(GL_TEXTURE0 + _texturesCount);
		_depthAttachment->AttachToFBO();
		_texturesCount++;
	}

	// does nothing right now
	void VFramebuffer::AddDepthRenderbufferAttachment() //TODO
	{
	}

	VGBufferFBO::VGBufferFBO(const glm::ivec2 & size) : VFramebuffer(size)
	{
		BindFBOWriting();
		AddColorAttachment(std::make_shared<VAlbedoTexture>(_size));
		AddColorAttachment(std::make_shared<VGeometryTexture>(_size));
		AddColorAttachment(std::make_shared<VGeometryTexture>(_size));
		AddDepthTextureAttachment();

		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
		};
		glDrawBuffers(3, drawBuffers);
		
		CheckStatus();

		UnbindFBOWriting();
	}

	VGBufferFBO::~VGBufferFBO()
	{
	}

	VShadowMap::VShadowMap(const glm::ivec2& size) : VFramebuffer(size)
	{
		BindFBOWriting();
		AddDepthTextureAttachment();
		UnbindFBOWriting();
	}

	void VShadowMap::BindTexturesReading()
	{
		_depthAttachment->ActivateTextureUnit();
		_depthAttachment->BindTexture();
	}

	void VShadowMap::UnbindTexturesReading()
	{
		_depthAttachment->ActivateTextureUnit();
		_depthAttachment->UnbindTexture();
	}

	VShadowMap::~VShadowMap()
	{

	}


	VShadowMapCube::VShadowMapCube(const glm::ivec2 & size) : VShadowMap(size)
	{
	}

	void VShadowMapCube::AddDepthTextureAttachment()
	{
	}

}