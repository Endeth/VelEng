#pragma once

#include <vector>
#include <memory>

#include "external/tiny_obj_loader.h"
#include "external/glm/common.hpp"
#include "vGeo/vGeo.h"
//#include "vGraphics/vShaders/vGLSLShader.h"

namespace Vel
{
	class BasicDrawableObject
	{
	public:
		BasicDrawableObject() {};
		virtual ~BasicDrawableObject() {};

		void Draw() {};
		void DrawWithImposedShader() {};
		//void SetShader(const std::shared_ptr<Shader>& Shd) { _shader = Shd; SetVAO(); };
		//const std::shared_ptr<Shader>& GetShader() const { return _shader; }

		size_t GetVerticesSize() { return _vertices.size(); }
		size_t GetIndicesSize() { return _indices.size(); }

		//virtual void SetUniformValue() = 0;
		virtual void SetVAO() = 0;

	//protected:
		virtual void BindAdditionalDrawingOptions() {};
		virtual void UnbindAdditionalDrawingOptions() {};

		std::vector<VertexUVColor> _vertices;
		std::vector<uint32_t> _indices;

		/*std::shared_ptr<Shader> _shader;
		template <typename ...Uniform>
		void SetUniformsValue( Uniform... Uniforms )
		{
			_shader->SetUniformsValue(Uniforms...);
		}*/
	};
}