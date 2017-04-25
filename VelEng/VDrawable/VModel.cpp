
#include "VModel.h"
#include "../VOpenGL/glm/gtc/matrix_transform.hpp"

namespace Vel
{
	using namespace std;


	Model::Model()
	{
	}

	Model::~Model()
	{
	}

	//draws meshes with their own shaders
	void Model::DrawModel()
	{
		SetModelMatrixUniform();
		for (auto &Mesh : _meshes)
		{
			Mesh->Draw();
		}
	}

	//draws meshes with a single given shader, eg. gPass
	void Model::DrawModelWithImposedShader()
	{
		for (auto &Mesh : _meshes)
		{
			//Mesh->DrawWithImposedShader();
			Mesh->DrawVerticesWithImposedShader(); //TODO
		}
	}

	void Model::AddMesh(const shared_ptr<Mesh> &mesh)
	{
		_meshes.push_back(mesh);
		_shaders.push_back(mesh->GetShader()); //TODO check for multiple instances of single shader
	}

	//sets internal model transformation matrix
	void Model::SetModelMatrix(const glm::mat4 & matrix)
	{
		_modelMatrix = matrix;
	}

	//sets model matrix in all of meshes shaders
	void Model::SetModelMatrixUniform()
	{
		for (auto &shader : _shaders)
		{
			shader->Activate();
			shader->SetUniformsValue(Uniform<glm::mat4>{ "M", _modelMatrix });
			shader->Deactivate();
		}
	}

	//sets model matrix in given shader
	void Model::SetModelMatrixUniform(const ShaderPtr & shader)
	{
		shader->SetUniformsValue(Uniform<glm::mat4>{ "M", _modelMatrix });
	}

	void Model::ModelMatrixTranslation(const glm::vec3& translation)
	{
		_modelMatrix = glm::translate(_modelMatrix, translation);
	}

	void Model::ModelMatrixScale(const glm::vec3& scale)
	{
		_modelMatrix = glm::scale(_modelMatrix, scale);
	}

	void Model::ModelMatrixRotation()
	{
		
	}


}
