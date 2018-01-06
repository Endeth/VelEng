#include "vSky.h"

namespace Vel
{
	Skybox::Skybox(const std::shared_ptr<TextureCube> &texture)
	{
		_skyTex = texture;
		LoadMesh();
	}

	void Skybox::SetVAO()
	{
		auto stride = sizeof(Vertex);

		glBindVertexArray(_vaoID);

		_vboVertices.BindBuffer();

		glEnableVertexAttribArray(_shader->GetAttribute("vVertex"));
		glVertexAttribPointer(_shader->GetAttribute("vVertex"), 3, GL_FLOAT, GL_FALSE, stride, 0);

		_vboIndices.BindBuffer();

		glBindVertexArray(0);
		_vboIndices.UnbindBuffer();
		_vboVertices.UnbindBuffer();
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