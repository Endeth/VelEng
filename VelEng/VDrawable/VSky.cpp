
#include "VSky.h"
namespace Vel
{
	VSkybox::VSkybox(const std::shared_ptr<VTextureCube> &texture)
	{
		_skyTex = texture;
		LoadMesh();
	}

	void VSkybox::SetVAO()
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
	void VSkybox::BindAdditionalDrawingOptions()
	{
		_skyTex->ActivateTextureUnit();
		_skyTex->BindTexture();
	}
	void VSkybox::UnbindAdditionalDrawingOptions()
	{
		_skyTex->UnbindTexture();
	}
}