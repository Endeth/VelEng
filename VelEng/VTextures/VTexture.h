#pragma once

#include "glew.h"
#include "glm/glm.hpp"
#include <string>

namespace Vel
{
	class VTexture
	{
	public:
		VTexture();
		VTexture(const glm::ivec2& size);
		VTexture(const std::string& path);
		virtual ~VTexture();
		void BindTexture();
		void ActivateTextureUnit();
		void UnbindTexture();
		void SetTextureUnit(GLuint unit = GL_TEXTURE0) { _textureUnit = unit; }
	protected:
		virtual void SetupTextureInfo();
		virtual void SetTextureParameters();
		virtual void LoadTexture(const std::string &Path);
		virtual void CreateTexture();

		glm::ivec2 _size;
		GLuint _texture;
		GLuint _wrapping;
		GLuint _filtering;
		GLuint _textureType;
		GLuint _textureUnit{ 0 };
	};

	class VSkyboxTexture : public VTexture
	{
	public:
		VSkyboxTexture(const std::string &Path);
	private:
		enum TexturePosition
		{
			POSX,
			NEGX,
			POSY,
			NEGY,
			POSZ,
			NEGZ,
		};
		virtual void SetupTextureInfo();
		virtual void SetTextureParameters();
		void LoadTexture(const std::string &Path);
		void LoadSingleTexture(const std::string &Path, const char *TexName, TexturePosition Pos);

		unsigned char* _texturePointer;

	};
}