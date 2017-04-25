#include "VDrawableObject.h"

namespace Vel
{
	using namespace std;

	BasicDrawableObject::BasicDrawableObject()
	{
		glGenVertexArrays(1, &_vaoID);
	}

	BasicDrawableObject::~BasicDrawableObject()
	{
		glDeleteVertexArrays(1, &_vaoID);
	}

	void BasicDrawableObject::Draw()
	{
		_shader->Activate();
		BindAdditionalDrawingOptions();
		glBindVertexArray(_vaoID);

		glDrawElements(_primitive, _indices.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		UnbindAdditionalDrawingOptions();
		_shader->Deactivate();
	}

	void BasicDrawableObject::DrawWithImposedShader()
	{
		BindAdditionalDrawingOptions();
		glBindVertexArray(_vaoID);

		glDrawElements(_primitive, _indices.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		UnbindAdditionalDrawingOptions();
	}

	void BasicDrawableObject::DrawVerticesWithImposedShader()
	{
		BindAdditionalDrawingOptions();
		glBindVertexArray(_vaoID);

		glDrawArrays(GL_TRIANGLES, 0, _vertices.size());

		glBindVertexArray(0);
		UnbindAdditionalDrawingOptions();
	}

}