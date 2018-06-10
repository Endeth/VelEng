#include "external/SOIL/SOIL.h"

#include "vFBOTextures.h"

namespace Vel
{
	FramebufferTexture::FramebufferTexture(const glm::ivec2& size)
	{
		_size = size;
		SetupTextureInfo();
		BindTexture();
		SetTextureParameters();
	}

	void FramebufferTexture::AttachToFBO(uint32_t attachment)
	{
	}

	void FramebufferTexture::SetupTextureInfo()
	{
	}

	void FramebufferTexture::SetTextureParameters()
	{
	}

	GeometryTexture::GeometryTexture(const glm::ivec2 & size) : FramebufferTexture(size)
	{
		CreateTexture();
		UnbindTexture();
	}

	void GeometryTexture::CreateTexture()
	{
	}

	AlbedoTexture::AlbedoTexture(const glm::ivec2 & size) : FramebufferTexture(size)
	{
		CreateTexture();
		UnbindTexture();
	}
	void AlbedoTexture::CreateTexture()
	{
	}

	void DepthTexture::AttachToFBO(uint32_t attachment)
	{
	}

	DepthTexture::DepthTexture(const glm::ivec2 & size) : FramebufferTexture(size)
	{
		CreateTexture();
		UnbindTexture();
	}

	void DepthTexture::CreateTexture()
	{
	}

	FramebufferTextureCube::FramebufferTextureCube(const glm::ivec2 & size)
	{
		_size = size; 
		SetupTextureInfo();
		BindTexture();
		SetTextureParameters();
	}

	void FramebufferTextureCube::AttachToFBO(uint32_t attachment)
	{
	}

	void FramebufferTextureCube::SetupTextureInfo()
	{
	}

	void FramebufferTextureCube::SetTextureParameters()
	{
	}

	DepthTextureCube::DepthTextureCube(const glm::ivec2 & size) : FramebufferTextureCube(size)
	{
		CreateTexture();
		UnbindTexture();
	}

	void DepthTextureCube::AttachToFBO(uint32_t attachment)
	{
		FramebufferTextureCube::AttachToFBO(attachment);
	}

	void DepthTextureCube::CreateTexture()
	{
	}


}