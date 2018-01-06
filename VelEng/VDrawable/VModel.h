#pragma once

#include <memory>
#include <vector>

#include "external/glm/glm.hpp"
#include "VMesh.h"

namespace Vel
{
	class Model
	{
		friend class Mesh;
		using ShaderPtr = std::shared_ptr<Shader>;
		using MeshPtr = std::shared_ptr<Mesh>;
	public:
		Model();
		//Model (filepath)
		//Model (model)
		virtual ~Model();

		void DrawModel();
		void DrawModelWithImposedShader();
		void AddMesh(const std::shared_ptr<Mesh> &mesh);

		void SetModelMatrix(const glm::mat4 &matrix);
		void SetModelMatrixUniform();
		void SetModelMatrixUniform(const ShaderPtr& shader);

		const glm::mat4& GetModelMatrix() const { return _modelMatrix; }

		void ModelMatrixTranslation(const glm::vec3& translation);
		void ModelMatrixScale(const glm::vec3& scale);
		void ModelMatrixRotation();

	private:
		std::vector<std::shared_ptr<Shader>> _shaders;
		std::vector<std::shared_ptr<Mesh>> _meshes;
		glm::mat4 _modelMatrix;
	};
}