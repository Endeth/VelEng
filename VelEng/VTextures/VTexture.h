#pragma once

#include "../VOpenGL/glew.h"
#include "../VOpenGL/glm/glm.hpp"
#include <string>

namespace Vel
{
	class VTexture
	{
	public:
		VTexture(const glm::ivec2& size);
		VTexture(const std::string& path);
		virtual ~VTexture();
		void BindTexture();
		void ActivateTextureUnit();
		void UnbindTexture();
		void SetTextureUnit(GLuint unit = GL_TEXTURE0) { _textureUnit = unit; }
	protected:
		VTexture();
		virtual void SetupTextureInfo();
		virtual void SetTextureParameters();
		virtual void LoadTexture(const std::string &Path);
		virtual void CreateEmptyTexture();

		glm::ivec2 _size;
		GLuint _texture;
		GLuint _wrapping;
		GLuint _filtering;
		GLuint _textureType;
		GLuint _textureUnit; 
		//TODO add internal format
	};

	class VTextureCube : public VTexture
	{
	public:
		VTextureCube(const glm::ivec2& size);
		VTextureCube(const std::string &Path);
	protected:
		VTextureCube();
		void CreateEmptyTexture() override;
		void SetupTextureInfo() override;
		void SetTextureParameters() override;
		void LoadTexture(const std::string &Path);
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

		void LoadSingleTexture(const std::string &Path, const char *TexName, TexturePosition Pos);

		//unsigned char* _texturePointer;

	};
}