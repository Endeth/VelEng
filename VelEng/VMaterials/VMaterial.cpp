
#include "../VOpenGL/glew.h"
#include "VMaterial.h"

namespace Vel
{


	using VTexturePtr = std::shared_ptr<Vel::VTexture>;

	VMaterial::VMaterial(const VTexturePtr& diffuse, const VTexturePtr& specular, GLfloat & shininess)
	{
		_diffuse = diffuse;
		_specular = specular;
		_shininess = shininess;
		SetTexturesUnits();
	}

	void VMaterial::SetTexturesUnits()
	{
		_diffuse->SetTextureUnit(GL_TEXTURE0);
		_specular->SetTextureUnit(GL_TEXTURE1);
	}

	void VMaterial::BindMaterial()
	{
		_diffuse->ActivateTextureUnit();
		_diffuse->BindTexture();
		_specular->ActivateTextureUnit();
		_specular->BindTexture();
	}

	void VMaterial::UnbindMaterial()
	{
		_diffuse->ActivateTextureUnit();
		_diffuse->UnbindTexture();
		_specular->ActivateTextureUnit();
		_specular->UnbindTexture();
	}

	VEmissiveMaterial::VEmissiveMaterial(const VTexturePtr& diffuse, const VTexturePtr& specular, const VTexturePtr& emission, GLfloat & shininess) : VMaterial(diffuse, specular, shininess)
	{
		_emission = emission;
		_emission->SetTextureUnit(GL_TEXTURE2);
	}


	void VEmissiveMaterial::BindMaterial()
	{
		VMaterial::BindMaterial();
		_emission->ActivateTextureUnit();
		_emission->BindTexture();
	}

	void VEmissiveMaterial::UnbindMaterial()
	{
		VMaterial::UnbindMaterial();
		_emission->ActivateTextureUnit();
		_emission->UnbindTexture();
	}
}
