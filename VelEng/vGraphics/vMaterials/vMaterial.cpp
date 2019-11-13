#include "vMaterial.h"

namespace Vel
{
	Material::Material()
	{
	}

	void Material::BindMaterial()
	{
		pipeline.Bind();
	}

	void Material::UnbindMaterial()
	{
		pipeline.Unbind();
	}

	PhongMaterial::PhongMaterial( const TexturePtr & diffuse, const TexturePtr & normalMap, const TexturePtr & specular, float & shininess )
	{
		_diffuse = diffuse;
		_normalMap = normalMap;
		_specular = specular;
		_shininess = shininess;
	}

	EmissiveMaterial::EmissiveMaterial(const TexturePtr& diffuse, const TexturePtr& normalMap, const TexturePtr& specular, const TexturePtr& emission, float & shininess) : PhongMaterial(diffuse, normalMap, specular, shininess) //TODO Normals and shininess not needed
	{
		_emission = emission;
	}

	void EmissiveMaterial::BindMaterial()
	{
		Material::BindMaterial();
		_emission->ActivateTextureUnit();
		_emission->BindTexture();
	}

	void EmissiveMaterial::UnbindMaterial()
	{
		Material::UnbindMaterial();
		_emission->ActivateTextureUnit();
		_emission->UnbindTexture();
	}
}
