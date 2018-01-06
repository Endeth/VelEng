#include "vFramebuffer.h"

namespace Vel
{
	Framebuffer::Framebuffer(const glm::ivec2& size) : _size(size)
	{
		glGenFramebuffers(1, &_fboID);
	}

	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers(1, &_fboID);
	}

	void Framebuffer::BindFBOWriting()
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fboID);
	}

	void Framebuffer::BindTexturesReading()
	{
		for (auto &tex : _colorTextures)
		{
			tex->ActivateTextureUnit();
			tex->BindTexture(); 
		}
		if (_depthAttachment != nullptr) //in case of renderbuffer
		{
			_depthAttachment->ActivateTextureUnit();
			_depthAttachment->BindTexture();
		}
	}

	void Framebuffer::UnbindFBOWriting()
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	void Framebuffer::UnbindTexturesReading()
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

	bool Framebuffer::CheckStatus()
	{
		auto Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE) 
		{
			printf("FB error, status: 0x%x\n", Status);
			return false;
		}
		return true;
	}

	void Framebuffer::AddColorAttachment(const TexturePtr & texture)
	{
		_colorTextures.push_back(texture);
		texture->SetTextureUnit(GL_TEXTURE0 + _texturesCount);
		texture->AttachToFBO(GL_COLOR_ATTACHMENT0 + _colorAttachments);
		_colorAttachments++;
		_texturesCount++;
	}

	//fixed precision
	void Framebuffer::AddDepthTextureAttachment()
	{
		_depthAttachment = std::make_shared<DepthTexture>(_size);
		_depthAttachment->SetTextureUnit(GL_TEXTURE0 + _texturesCount);
		_depthAttachment->AttachToFBO();
		_texturesCount++;
	}

	// does nothing right now
	void Framebuffer::AddDepthRenderbufferAttachment() //TODO
	{
	}

	GBufferFBO::GBufferFBO(const glm::ivec2 & size) : Framebuffer(size)
	{
		BindFBOWriting();
		AddColorAttachment(std::make_shared<AlbedoTexture>(_size));
		AddColorAttachment(std::make_shared<GeometryTexture>(_size));
		AddColorAttachment(std::make_shared<GeometryTexture>(_size));
		AddDepthTextureAttachment();

		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
		};
		glDrawBuffers(3, drawBuffers);
		
		CheckStatus();

		UnbindFBOWriting();
	}

	GBufferFBO::~GBufferFBO()
	{
	}

	ShadowMap2D::ShadowMap2D(const glm::ivec2& size) : Framebuffer(size)
	{
		_texturesCount = 4; //TODO erase this nasty hack
		BindFBOWriting();
		AddDepthTextureAttachment();
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		UnbindFBOWriting();
	}

	void ShadowMap2D::BindTexturesReading()
	{
		_depthAttachment->ActivateTextureUnit();
		_depthAttachment->BindTexture();
	}

	void ShadowMap2D::UnbindTexturesReading()
	{
		_depthAttachment->ActivateTextureUnit();
		_depthAttachment->UnbindTexture();
	}

	void ShadowMap2D::SetTextureUnit(GLuint textureUnit)
	{
		_depthAttachment->SetTextureUnit(textureUnit);
	}

	ShadowMap2D::~ShadowMap2D()
	{

	}


	ShadowMapCube::ShadowMapCube(const glm::ivec2 & size) : Framebuffer(size)
	{
		BindFBOWriting();
		AddDepthTextureAttachment();
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		CheckStatus();
		UnbindFBOWriting();
	}

	void ShadowMapCube::AddDepthTextureAttachment()
	{
		_texturesCount = 4;
		_depthAttachment = std::make_shared<DepthTextureCube>(_size);
		_depthAttachment->SetTextureUnit(GL_TEXTURE0 + _texturesCount);
		_depthAttachment->AttachToFBO(GL_DEPTH_ATTACHMENT);
		_texturesCount++;
	}

	void ShadowMapCube::BindTexturesReading()
	{
		_depthAttachment->ActivateTextureUnit();
		_depthAttachment->BindTexture();
	}

	void ShadowMapCube::UnbindTexturesReading()
	{
		_depthAttachment->ActivateTextureUnit();
		_depthAttachment->UnbindTexture();
	}

	void ShadowMapCube::SetTextureUnit(GLuint textureUnit)
	{
		_depthAttachment->SetTextureUnit(textureUnit);
	}

}