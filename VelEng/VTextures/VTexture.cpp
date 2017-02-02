
#include "VTexture.h"
#include "SOIL.h"

namespace Vel
{
	VTexture::VTexture()
	{
		glGenTextures(1, &_texture);
	}
	VTexture::VTexture(const glm::ivec2& size) : _size(size)
	{
		glGenTextures(1, &_texture);
		SetupTextureInfo();
		BindTexture();
		SetTextureParameters();
		CreateEmptyTexture();
		UnbindTexture();

	}

	VTexture::VTexture(const std::string &path)
	{
		glGenTextures(1, &_texture);
		SetupTextureInfo();
		BindTexture();
		SetTextureParameters();
		LoadTexture(path);
		UnbindTexture();
	}

	VTexture::~VTexture()
	{
		glDeleteTextures(1, &_texture);
	}

	void VTexture::SetupTextureInfo()
	{
		_textureType = GL_TEXTURE_2D;
		_wrapping = GL_MIRRORED_REPEAT;
		_filtering = GL_LINEAR;
	}

	void VTexture::SetTextureParameters()
	{
		glTexParameteri(_textureType, GL_TEXTURE_MIN_FILTER, _filtering);
		glTexParameteri(_textureType, GL_TEXTURE_MAG_FILTER, _filtering);

		glTexParameteri(_textureType, GL_TEXTURE_WRAP_S, _wrapping);
		glTexParameteri(_textureType, GL_TEXTURE_WRAP_T, _wrapping);
	}

	void VTexture::LoadTexture(const std::string &Path)
	{
		auto img = SOIL_load_image(Path.c_str(), &(_size.x), &(_size.y), 0, SOIL_LOAD_RGB);
		glTexImage2D(_textureType, 0, GL_RGB, _size.x, _size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
		SOIL_free_image_data(img);
	}

	void VTexture::CreateEmptyTexture()
	{
		glTexImage2D(_textureType, 0, GL_RGB, _size.x, _size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	}

	void VTexture::BindTexture()
	{
		glBindTexture(_textureType, _texture);
	}

	void VTexture::ActivateTextureUnit()
	{
		glActiveTexture(_textureUnit);
	}

	void VTexture::UnbindTexture()
	{
		glBindTexture(_textureType, 0);
	}


	VTextureCube::VTextureCube()
	{
	}

	VTextureCube::VTextureCube(const glm::ivec2 & size)
	{
		SetupTextureInfo();
		SetTextureUnit();
		BindTexture();
		SetTextureParameters();
		CreateEmptyTexture();
		UnbindTexture();
	}

	VTextureCube::VTextureCube(const std::string &Path)
	{
		SetupTextureInfo();
		SetTextureUnit();
		BindTexture();
		SetTextureParameters();
		LoadTexture(Path);
		UnbindTexture();
	}


	void VTextureCube::LoadTexture(const std::string &Path)
	{
		LoadSingleTexture(Path, "//posx.png", POSX);
		LoadSingleTexture(Path, "//negx.png", NEGX);

		LoadSingleTexture(Path, "//posy.png", POSY);
		LoadSingleTexture(Path, "//negy.png", NEGY);

		LoadSingleTexture(Path, "//posz.png", POSZ);
		LoadSingleTexture(Path, "//negz.png", NEGZ);
	}

	void VTextureCube::CreateEmptyTexture()
	{
		for(GLuint i = 0; i < 6; i++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, _size.x, _size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	}

	void VTextureCube::LoadSingleTexture(const std::string &Path, const char* TexName, TexturePosition Pos)
	{
		auto singleTexPath = Path + TexName;
		unsigned char* _texturePointer = SOIL_load_image(singleTexPath.c_str(), &(_size.x), &(_size.y), 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Pos, 0, GL_RGB, _size.x, _size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, _texturePointer);
		SOIL_free_image_data(_texturePointer);
	}
	void VTextureCube::SetupTextureInfo()
	{
		_textureType = GL_TEXTURE_CUBE_MAP;
		_wrapping = GL_CLAMP_TO_EDGE;
		_filtering = GL_LINEAR;
	}
	void VTextureCube::SetTextureParameters()
	{
		glTexParameteri(_textureType, GL_TEXTURE_MIN_FILTER, _filtering);
		glTexParameteri(_textureType, GL_TEXTURE_MAG_FILTER, _filtering);

		glTexParameteri(_textureType, GL_TEXTURE_WRAP_S, _wrapping);
		glTexParameteri(_textureType, GL_TEXTURE_WRAP_T, _wrapping);
		glTexParameteri(_textureType, GL_TEXTURE_WRAP_R, _wrapping);
	}

}