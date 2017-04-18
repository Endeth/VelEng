
#include "VFBOTextures.h"
#include "SOIL.h"

namespace Vel
{
	FramebufferTexture::FramebufferTexture(const glm::ivec2& size)
	{
		_size = size;
		SetupTextureInfo();
		BindTexture();
		SetTextureParameters();
	}

	void FramebufferTexture::AttachToFBO(GLuint attachment)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, _textureType, _texture, 0);
	}

	void FramebufferTexture::SetupTextureInfo()
	{
		_textureType = GL_TEXTURE_2D;
		_wrapping = GL_CLAMP_TO_EDGE;
		_filtering = GL_LINEAR;
	}

	void FramebufferTexture::SetTextureParameters()
	{
		glTexParameteri(_textureType, GL_TEXTURE_MIN_FILTER, _filtering);
		glTexParameteri(_textureType, GL_TEXTURE_MAG_FILTER, _filtering);

		glTexParameteri(_textureType, GL_TEXTURE_WRAP_S, _wrapping);
		glTexParameteri(_textureType, GL_TEXTURE_WRAP_T, _wrapping);

		GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	}

	GeometryTexture::GeometryTexture(const glm::ivec2 & size) : FramebufferTexture(size)
	{
		CreateTexture();
		UnbindTexture();
	}

	void GeometryTexture::CreateTexture()
	{
		glTexImage2D(_textureType, 0, GL_RGB16F, _size.x, _size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}

	AlbedoTexture::AlbedoTexture(const glm::ivec2 & size) : FramebufferTexture(size)
	{
		CreateTexture();
		UnbindTexture();
	}
	void AlbedoTexture::CreateTexture()
	{
		glTexImage2D(_textureType, 0, GL_RGBA32F, _size.x, _size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	void DepthTexture::AttachToFBO(GLuint attachment)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, _textureType, _texture, 0);
	}

	DepthTexture::DepthTexture(const glm::ivec2 & size) : FramebufferTexture(size)
	{
		CreateTexture();
		UnbindTexture();
	}

	void DepthTexture::CreateTexture()
	{
		glTexImage2D(_textureType, 0, GL_DEPTH_COMPONENT, _size.x, _size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	FramebufferTextureCube::FramebufferTextureCube(const glm::ivec2 & size)
	{
		_size = size; 
		SetupTextureInfo();
		BindTexture();
		SetTextureParameters();
	}

	void FramebufferTextureCube::AttachToFBO(GLuint attachment)
	{
		glFramebufferTexture(GL_FRAMEBUFFER, attachment, _texture, 0);
	}

	void FramebufferTextureCube::SetupTextureInfo()
	{
		_textureType = GL_TEXTURE_CUBE_MAP;
		_wrapping = GL_CLAMP_TO_EDGE;
		_filtering = GL_NEAREST;
	}

	void FramebufferTextureCube::SetTextureParameters()
	{
		glTexParameteri(_textureType, GL_TEXTURE_MIN_FILTER, _filtering);
		glTexParameteri(_textureType, GL_TEXTURE_MAG_FILTER, _filtering);

		glTexParameteri(_textureType, GL_TEXTURE_WRAP_S, _wrapping);
		glTexParameteri(_textureType, GL_TEXTURE_WRAP_T, _wrapping);
		glTexParameteri(_textureType, GL_TEXTURE_WRAP_R, _wrapping);

		GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(_textureType, GL_TEXTURE_BORDER_COLOR, borderColor);
	}

	DepthTextureCube::DepthTextureCube(const glm::ivec2 & size) : FramebufferTextureCube(size)
	{
		CreateTexture();
		UnbindTexture();
	}

	void DepthTextureCube::AttachToFBO(GLuint attachment)
	{
		FramebufferTextureCube::AttachToFBO(attachment);
	}

	void DepthTextureCube::CreateTexture()
	{
		for (GLuint i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
				_size.x, _size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}


}