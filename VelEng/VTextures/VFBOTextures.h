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
		void SetupTextureInfo();
		void SetTextureParameters();
	};

	class VGeometryTexture : public VFramebufferTexture
	{
	public:
		VGeometryTexture(const glm::ivec2& size);
	private:
		virtual void CreateTexture();
	};

	class VAlbedoTexture : public VFramebufferTexture
	{
	public:
		VAlbedoTexture(const glm::ivec2& size);
	private:
		virtual void CreateTexture();
	};

	class VDepthTexture : public VFramebufferTexture
	{
	public:
		void AttachToFBO(GLuint attachment = GL_DEPTH_ATTACHMENT);
		VDepthTexture(const glm::ivec2& size);
		GLuint GetID() { return _texture; }
	private:
		virtual void CreateTexture();
	};
}