
#include "../VOpenGL/glew.h"
#include "VMaterial.h"

namespace Vel
{


	

	Material::Material(const VTexturePtr& diffuse, const VTexturePtr& specular, GLfloat & shininess)
	{
		_diffuse = diffuse;
		_specular = specular;
		_shininess = shininess;
		SetTexturesUnits();
	}

	void Material::SetTexturesUnits()
	{
		_diffuse->SetTextureUnit(GL_TEXTURE0);
		_specular->SetTextureUnit(GL_TEXTURE1);
	}

	void Material::BindMaterial()
	{
		_diffuse->ActivateTextureUnit();
		_diffuse->BindTexture();
		_specular->ActivateTextureUnit();
		_specular->BindTexture();
	}

	void Material::UnbindMaterial()
	{
		_diffuse->ActivateTextureUnit();
		_diffuse->UnbindTexture();
		_specular->ActivateTextureUnit();
		_specular->UnbindTexture();
	}

	EmissiveMaterial::EmissiveMaterial(const VTexturePtr& diffuse, const VTexturePtr& specular, const VTexturePtr& emission, GLfloat & shininess) : Material(diffuse, specular, shininess)
	{
		_emission = emission;
		_emission->SetTextureUnit(GL_TEXTURE2);
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
