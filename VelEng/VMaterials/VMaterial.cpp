
#include "../VOpenGL/glew.h"
#include "VMaterial.h"

namespace Vel
{

    //TODO make basic material? only diffuse

	Material::Material(const VTexturePtr& diffuse, const VTexturePtr& normalMap, const VTexturePtr& specular, GLfloat & shininess)
	{
		_diffuse = diffuse;
        _normalMap = normalMap;
		_specular = specular;
		_shininess = shininess;
		SetTexturesUnits();
	}

	void Material::SetTexturesUnits()
	{
		_diffuse->SetTextureUnit(GL_TEXTURE0);
		_normalMap->SetTextureUnit(GL_TEXTURE1);
		_specular->SetTextureUnit(GL_TEXTURE2);
	}

	void Material::BindMaterial()
	{
		_diffuse->ActivateTextureUnit();
		_diffuse->BindTexture();
        _normalMap->ActivateTextureUnit();
        _normalMap->BindTexture();
		_specular->ActivateTextureUnit();
		_specular->BindTexture();
	}

	void Material::UnbindMaterial()
	{
		_diffuse->ActivateTextureUnit();
		_diffuse->UnbindTexture();
        _normalMap->ActivateTextureUnit();
        _normalMap->UnbindTexture();
		_specular->ActivateTextureUnit();
		_specular->UnbindTexture();
	}

	EmissiveMaterial::EmissiveMaterial(const VTexturePtr& diffuse, const VTexturePtr& normalMap, const VTexturePtr& specular, const VTexturePtr& emission, GLfloat & shininess) : Material(diffuse, normalMap, specular, shininess) //TODO Normals and shininess not needed
	{
		_emission = emission;
		_emission->SetTextureUnit(GL_TEXTURE3);
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
