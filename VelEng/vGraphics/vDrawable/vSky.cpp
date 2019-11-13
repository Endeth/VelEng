#include "vSky.h"

namespace Vel
{
	Skybox::Skybox(const std::shared_ptr<TextureCube> &texture)
	{
		_skyTex = texture;
		//LoadMesh( "assets/cube.obj" );
	}

	void Skybox::SetVAO()
	{
	}
	void Skybox::BindAdditionalDrawingOptions()
	{
		_skyTex->ActivateTextureUnit();
		_skyTex->BindTexture();
	}
	void Skybox::UnbindAdditionalDrawingOptions()
	{
		_skyTex->UnbindTexture();
	}
}