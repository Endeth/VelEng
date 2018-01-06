#pragma once

#include "vTexture.h"

namespace Vel
{
	class FramebufferTexture : public Texture
	{
	public:
		void AttachToFBO(GLuint attachment);
	protected:
		FramebufferTexture(const glm::ivec2& size);
		void SetupTextureInfo() override;
		void SetTextureParameters() override;
	};

	class GeometryTexture : public FramebufferTexture
	{
	public:
		GeometryTexture(const glm::ivec2& size);
	private:
		void CreateTexture();
	};

	class AlbedoTexture : public FramebufferTexture
	{
	public:
		AlbedoTexture(const glm::ivec2& size);
	private:
		void CreateTexture();
	};

	class DepthTexture : public FramebufferTexture
	{
	public:
		DepthTexture(const glm::ivec2& size = glm::ivec2(512,512));
		void AttachToFBO(GLuint attachment = GL_DEPTH_ATTACHMENT);
	private:
		void CreateTexture();
	};

	class FramebufferTextureCube : public TextureCube //TODO textures SUCK need to rethink their abstraction
	{
	public:
		void AttachToFBO(GLuint attachment);
	protected:
		FramebufferTextureCube(const glm::ivec2& size);
		void SetupTextureInfo() override;
		void SetTextureParameters() override;
	};

	class DepthTextureCube : public FramebufferTextureCube
	{
	public:
		DepthTextureCube(const glm::ivec2& size = glm::ivec2(512, 512));
		void AttachToFBO(GLuint attachment = GL_DEPTH_ATTACHMENT);
	private:
		void CreateTexture();
	};
}