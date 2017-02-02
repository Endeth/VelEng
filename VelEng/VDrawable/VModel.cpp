
#include "VModel.h"
#include "glm/gtc/matrix_transform.hpp"

namespace Vel
{
	using namespace std;


	VModel::VModel()
	{
	}

	VModel::~VModel()
	{
	}

	//draws meshes with their own shaders
	void VModel::DrawModel()
	{
		SetModelMatrixUniform();
		for (auto &Mesh : _meshes)
		{
			Mesh->Draw();
		}
	}

	//draws meshes with a single given shader, eg. gPass
	void VModel::DrawModelWithImposedShader()
	{
		for (auto &Mesh : _meshes)
		{
			//Mesh->DrawWithImposedShader();
			Mesh->DrawVerticesWithImposedShader();
		}
	}

	void VModel::AddMesh(const shared_ptr<VMesh> &mesh)
	{
		_meshes.push_back(mesh);
		_shaders.push_back(mesh->GetShader()); //TODO check for multiple instances of single shader
	}

	//sets internal model transformation matrix
	void VModel::SetModelMatrix(const glm::mat4 & matrix)
	{
		_modelMatrix = matrix;
	}

	//sets model matrix in all of meshes shaders
	void VModel::SetModelMatrixUniform()
	{
		for (auto &shader : _shaders)
		{
			shader->Activate();
			shader->SetUniformsValue(Uniform<glm::mat4>{ "M", _modelMatrix });
			shader->Deactivate();
		}
	}

	//sets model matrix in given shader
	void VModel::SetModelMatrixUniform(const ShaderPtr & shader)
	{
		shader->SetUniformsValue(Uniform<glm::mat4>{ "M", _modelMatrix });
	}

	void VModel::ModelMatrixTranslation(const glm::vec3& translation)
	{
		_modelMatrix = glm::translate(_modelMatrix, translation);
	}

	void VModel::ModelMatrixScale(const glm::vec3& scale)
	{
		_modelMatrix = glm::scale(_modelMatrix, scale);
	}

	void VModel::ModelMatrixRotation()
	{
		
	}


}
