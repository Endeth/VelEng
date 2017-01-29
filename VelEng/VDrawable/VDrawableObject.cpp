#include "VDrawableObject.h"

namespace Vel
{
	using namespace std;

	VBasicDrawableObject::VBasicDrawableObject()
	{
		glGenVertexArrays(1, &_vaoID);
	}

	VBasicDrawableObject::~VBasicDrawableObject()
	{

		glDeleteVertexArrays(1, &_vaoID);
	}

	void VBasicDrawableObject::Draw()
	{
		_shader->Activate();
		BindAdditionalDrawingOptions();
		glBindVertexArray(_vaoID);

		glDrawElements(_primitive, _indices.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		UnbindAdditionalDrawingOptions();
		_shader->Deactivate();
	}

	void VBasicDrawableObject::DrawWithImposedShader()
	{
		BindAdditionalDrawingOptions();
		glBindVertexArray(_vaoID);

		glDrawElements(_primitive, _indices.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		UnbindAdditionalDrawingOptions();
	}

}