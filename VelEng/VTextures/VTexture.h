#pragma once

#include "../VOpenGL/glew.h"
#include "../VOpenGL/glm/glm.hpp"
#include <string>

namespace Vel
{
	//TODO rewrite this
	class Texture
	{
	public:
		Texture(const glm::ivec2& size);
		Texture(const std::string& path);
		virtual ~Texture();
		void BindTexture();
		void ActivateTextureUnit();
		void UnbindTexture();
		void SetTextureUnit(GLuint unit = GL_TEXTURE0) { _textureUnit = unit; }
	protected:
		Texture();
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

	class TextureCube : public Texture
	{
	public:
		TextureCube(const glm::ivec2& size);
		TextureCube(const std::string &Path);
	protected:
		TextureCube();
		virtual void CreateEmptyTexture() override;
		virtual void SetupTextureInfo() override;
		virtual void SetTextureParameters() override;
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
	};
}