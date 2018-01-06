#pragma once

#include <vector>
#include <memory>

#include "external/glm/common.hpp"
#include "vShaders/vGLSLShader.h"
#include "vGeo/vGeo.h"

namespace Vel
{
	class BasicDrawableObject
	{
	public:
		BasicDrawableObject();
		virtual ~BasicDrawableObject();

		void Draw();
		void DrawWithImposedShader();
		void DrawVerticesWithImposedShader(); //DEBUG
		void SetShader(const std::shared_ptr<Shader>& Shd) { _shader = Shd; SetVAO(); };
		const std::shared_ptr<Shader>& GetShader() const { return _shader; }

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

		std::shared_ptr<Shader> _shader;
	};
}