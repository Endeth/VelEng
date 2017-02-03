#pragma once
#include "VTexture.h"

namespace Vel
{
	class VFramebufferTexture : public VTexture
	{
	public:
		void AttachToFBO(GLuint attachment);
	protected:
		VFramebufferTexture(const glm::ivec2& size);
		void SetupTextureInfo() override;
		void SetTextureParameters() override;
	};

	class VGeometryTexture : public VFramebufferTexture
	{
	public:
		VGeometryTexture(const glm::ivec2& size);
	private:
		void CreateTexture();
	};

	class VAlbedoTexture : public VFramebufferTexture
	{
	public:
		VAlbedoTexture(const glm::ivec2& size);
	private:
		void CreateTexture();
	};

	class VDepthTexture : public VFramebufferTexture
	{
	public:
		VDepthTexture(const glm::ivec2& size = glm::ivec2(512,512));
		void AttachToFBO(GLuint attachment = GL_DEPTH_ATTACHMENT);
	private:
		void CreateTexture();
	};

	class VFramebufferTextureCube : public VTextureCube //TODO textures SUCK need to rethink their abstraction
	{
	public:
		void AttachToFBO(GLuint attachment);
	protected:
		VFramebufferTextureCube(const glm::ivec2& size);
		void SetupTextureInfo() override;
		void SetTextureParameters() override;
	};

	class VDepthTextureCube : public VFramebufferTextureCube
	{
	public:
		VDepthTextureCube(const glm::ivec2& size = glm::ivec2(512, 512));
		void AttachToFBO(GLuint attachment = GL_DEPTH_ATTACHMENT);
	private:
		void CreateTexture();
	};
}