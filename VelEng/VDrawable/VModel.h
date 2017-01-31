#pragma once

#include "glew.h"
#include "glm/glm.hpp"
#include "VMesh.h"
#include <memory>
#include <vector>

namespace Vel
{
	class VModel
	{
		friend class VMesh;
		using ShaderPtr = std::shared_ptr<VGLSLShader>;
		using MeshPtr = std::shared_ptr<VMesh>;
	public:
		VModel();
		//Model (filepath)
		//Model (model)
		virtual ~VModel();

		void DrawModel();
		void DrawModelWithImposedShader();
		void AddMesh(const std::shared_ptr<VMesh> &mesh);

		void SetModelMatrix(const glm::mat4 &matrix);
		void SetModelMatrixUniform();
		void SetModelMatrixUniform(const ShaderPtr& shader);

		const glm::mat4& GetModelMatrix() const { return _modelMatrix; }

		void ModelMatrixTranslation(const glm::vec3& translation);
		void ModelMatrixScale(const glm::vec3& scale);
		void ModelMatrixRotation();

	private:
		std::vector<std::shared_ptr<VGLSLShader>> _shaders;
		std::vector<std::shared_ptr<VMesh>> _meshes;
		glm::mat4 _modelMatrix;
	};
}