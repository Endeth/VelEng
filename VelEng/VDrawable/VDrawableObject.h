#pragma once

#include <vector>
#include "GL/glew.h"
#include "glm/common.hpp"
#include "VGLSLShader.h"
#include "VGeo.h"
#include <memory>

namespace Vel
{
	class VBasicDrawableObject
	{
	public:
		VBasicDrawableObject();
		virtual ~VBasicDrawableObject();

		void Draw();
		void DrawWithImposedShader();
		void SetShader(const std::shared_ptr<GLSLShader>& Shd) { _shader = Shd; SetVAO(); };
		const std::shared_ptr<GLSLShader>& GetShader() const { return _shader; }

		//virtual void SetUniformValue() = 0;
		virtual void SetVAO() = 0;

	protected:

		template <typename ...Uniform>
		void SetUniformsValue(Uniform... Uniforms)
		{
			_shader->SetUniformsValue(Uniforms...);
		}
		virtual void BindAdditionalDrawingOptions() {};
		virtual void UnbindAdditionalDrawingOptions() {};

		std::vector<Vertex> _vertices;
		std::vector<GLuint> _indices;
		GLuint _vaoID;
		GLenum _primitive;

		std::shared_ptr<GLSLShader> _shader;
	};
}