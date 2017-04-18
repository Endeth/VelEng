
#include "VTexture.h"
#include "SOIL.h"

namespace Vel
{
	Texture::Texture()
	{
		glGenTextures(1, &_texture);
	}
	Texture::Texture(const glm::ivec2& size) : _size(size)
	{
		glGenTextures(1, &_texture);
		SetupTextureInfo();
		BindTexture();
		SetTextureParameters();
		CreateEmptyTexture();
		UnbindTexture();

	}

	Texture::Texture(const std::string &path)
	{
		glGenTextures(1, &_texture);
		SetupTextureInfo();
		BindTexture();
		SetTextureParameters();
		LoadTexture(path);
		UnbindTexture();
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &_texture);
	}

	void Texture::SetupTextureInfo()
	{
		_textureType = GL_TEXTURE_2D;
		_wrapping = GL_MIRRORED_REPEAT;
		_filtering = GL_LINEAR;
	}

	void Texture::SetTextureParameters()
	{
		glTexParameteri(_textureType, GL_TEXTURE_MIN_FILTER, _filtering);
		glTexParameteri(_textureType, GL_TEXTURE_MAG_FILTER, _filtering);

		glTexParameteri(_textureType, GL_TEXTURE_WRAP_S, _wrapping);
		glTexParameteri(_textureType, GL_TEXTURE_WRAP_T, _wrapping);
	}

	void Texture::LoadTexture(const std::string &Path)
	{
		auto img = SOIL_load_image(Path.c_str(), &(_size.x), &(_size.y), 0, SOIL_LOAD_RGB);
		glTexImage2D(_textureType, 0, GL_RGB, _size.x, _size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
		SOIL_free_image_data(img);
	}

	void Texture::CreateEmptyTexture()
	{
		glTexImage2D(_textureType, 0, GL_RGB, _size.x, _size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	}

	void Texture::BindTexture()
	{
		glBindTexture(_textureType, _texture);
	}

	void Texture::ActivateTextureUnit()
	{
		glActiveTexture(_textureUnit);
	}

	void Texture::UnbindTexture()
	{
		glBindTexture(_textureType, 0);
	}


	TextureCube::TextureCube()
	{
	}

	TextureCube::TextureCube(const glm::ivec2 & size)
	{
		SetupTextureInfo();
		SetTextureUnit();
		BindTexture();
		SetTextureParameters();
		CreateEmptyTexture();
		UnbindTexture();
	}

	TextureCube::TextureCube(const std::string &Path)
	{
		SetupTextureInfo();
		SetTextureUnit();
		BindTexture();
		SetTextureParameters();
		LoadTexture(Path);
		UnbindTexture();
	}


	void TextureCube::LoadTexture(const std::string &Path)
	{
		LoadSingleTexture(Path, "//posx.png", POSX);
		LoadSingleTexture(Path, "//negx.png", NEGX);

		LoadSingleTexture(Path, "//posy.png", POSY);
		LoadSingleTexture(Path, "//negy.png", NEGY);

		LoadSingleTexture(Path, "//posz.png", POSZ);
		LoadSingleTexture(Path, "//negz.png", NEGZ);
	}

	void TextureCube::CreateEmptyTexture()
	{
		for(GLuint i = 0; i < 6; i++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, _size.x, _size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	}

	void TextureCube::LoadSingleTexture(const std::string &Path, const char* TexName, TexturePosition Pos)
	{
		auto singleTexPath = Path + TexName;
		unsigned char* _texturePointer = SOIL_load_image(singleTexPath.c_str(), &(_size.x), &(_size.y), 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Pos, 0, GL_RGB, _size.x, _size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, _texturePointer);
		SOIL_free_image_data(_texturePointer);
	}
	void TextureCube::SetupTextureInfo()
	{
		_textureType = GL_TEXTURE_CUBE_MAP;
		_wrapping = GL_CLAMP_TO_EDGE;
		_filtering = GL_LINEAR;
	}
	void TextureCube::SetTextureParameters()
	{
		glTexParameteri(_textureType, GL_TEXTURE_MIN_FILTER, _filtering);
		glTexParameteri(_textureType, GL_TEXTURE_MAG_FILTER, _filtering);

		glTexParameteri(_textureType, GL_TEXTURE_WRAP_S, _wrapping);
		glTexParameteri(_textureType, GL_TEXTURE_WRAP_T, _wrapping);
		glTexParameteri(_textureType, GL_TEXTURE_WRAP_R, _wrapping);
	}

}