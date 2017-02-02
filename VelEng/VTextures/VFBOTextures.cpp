
#include "VFBOTextures.h"
#include "SOIL.h"

namespace Vel
{
	VFramebufferTexture::VFramebufferTexture(const glm::ivec2& size)
	{
		_size = size;
		SetupTextureInfo();
		BindTexture();
		SetTextureParameters();
	}

	void VFramebufferTexture::AttachToFBO(GLuint attachment)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, _textureType, _texture, 0);
	}

	void VFramebufferTexture::SetupTextureInfo()
	{
		_textureType = GL_TEXTURE_2D;
		_wrapping = GL_CLAMP_TO_EDGE;
		_filtering = GL_LINEAR;
	}

	void VFramebufferTexture::SetTextureParameters()
	{
		glTexParameteri(_textureType, GL_TEXTURE_MIN_FILTER, _filtering);
		glTexParameteri(_textureType, GL_TEXTURE_MAG_FILTER, _filtering);

		glTexParameteri(_textureType, GL_TEXTURE_WRAP_S, _wrapping);
		glTexParameteri(_textureType, GL_TEXTURE_WRAP_T, _wrapping);

		GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	}

	VGeometryTexture::VGeometryTexture(const glm::ivec2 & size) : VFramebufferTexture(size)
	{
		CreateTexture();
		UnbindTexture();
	}

	void VGeometryTexture::CreateTexture()
	{
		glTexImage2D(_textureType, 0, GL_RGB16F, _size.x, _size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}

	VAlbedoTexture::VAlbedoTexture(const glm::ivec2 & size) : VFramebufferTexture(size)
	{
		CreateTexture();
		UnbindTexture();
	}
	void VAlbedoTexture::CreateTexture()
	{
		glTexImage2D(_textureType, 0, GL_RGBA32F, _size.x, _size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	void VDepthTexture::AttachToFBO(GLuint attachment)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, _textureType, _texture, 0);
	}

	VDepthTexture::VDepthTexture(const glm::ivec2 & size) : VFramebufferTexture(size)
	{
		CreateTexture();
		UnbindTexture();
	}

	void VDepthTexture::CreateTexture()
	{
		glTexImage2D(_textureType, 0, GL_DEPTH_COMPONENT32, _size.x, _size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	VFramebufferTextureCube::VFramebufferTextureCube(const glm::ivec2 & size)
	{
		_size = size;
	}

	void VFramebufferTextureCube::AttachToFBO(GLuint attachment)
	{

	}

	void VFramebufferTextureCube::SetupTextureInfo()
	{
		_textureType = GL_TEXTURE_CUBE_MAP;
		_wrapping = GL_CLAMP_TO_EDGE;
		_filtering = GL_LINEAR;
	}

	void VFramebufferTextureCube::SetTextureParameters()
	{
		glTexParameteri(_textureType, GL_TEXTURE_MIN_FILTER, _filtering);
		glTexParameteri(_textureType, GL_TEXTURE_MAG_FILTER, _filtering);

		glTexParameteri(_textureType, GL_TEXTURE_WRAP_S, _wrapping);
		glTexParameteri(_textureType, GL_TEXTURE_WRAP_T, _wrapping);

		GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	}

	VDepthTextureCube::VDepthTextureCube(const glm::ivec2 & size) : VFramebufferTextureCube(size)
	{
		SetupTextureInfo();
		BindTexture();
		SetTextureParameters();
	}

	void VDepthTextureCube::AttachToFBO(GLuint attachment)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, _textureType, _texture, 0);
	}

	void VDepthTextureCube::CreateTexture()
	{
		glTexImage2D(_textureType, 0, GL_DEPTH_COMPONENT32, _size.x, _size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	void VDepthTextureCube::SetupTextureInfo()
	{
		_textureType = GL_TEXTURE_2D;
		_wrapping = GL_CLAMP_TO_EDGE;
		_filtering = GL_LINEAR;
	}

}