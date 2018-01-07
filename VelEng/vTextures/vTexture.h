#pragma once

#include <string>

#include "external/glm/glm.hpp"

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
		void SetTextureUnit(uint32_t unit = 0) { _textureUnit = unit; }
	protected:
		Texture();
		virtual void SetupTextureInfo();
		virtual void SetTextureParameters();
		virtual void LoadTexture(const std::string &Path);
		virtual void CreateEmptyTexture();

		glm::ivec2 _size;
		uint32_t _texture;
		uint32_t _wrapping;
		uint32_t _filtering;
		uint32_t _textureType;
		uint32_t _textureUnit; 
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