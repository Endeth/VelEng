
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
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, _texture, 0);
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
		glTexImage2D(_textureType, 0, GL_RGB32F, _size.x, _size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}

	VAlbedoTexture::VAlbedoTexture(const glm::ivec2 & size) : VFramebufferTexture(size)
	{
		CreateTexture();
		UnbindTexture();
	}
	void VAlbedoTexture::CreateTexture()
	{
		glTexImage2D(_textureType, 0, GL_RGBA, _size.x, _size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	void VDepthTexture::AttachToFBO(GLuint attachment)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, _texture, 0);
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

	void VDepthTexture::SetupTextureInfo()
	{
		_textureType = GL_TEXTURE_2D;
		_wrapping = GL_MIRRORED_REPEAT;
		_filtering = GL_LINEAR;
	}

}