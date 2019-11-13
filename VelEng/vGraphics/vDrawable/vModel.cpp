#include <algorithm>
#include "external/glm/gtc/matrix_transform.hpp"
#include "vModel.h"

static const char* materialsDir = "assets/";

namespace Vel
{
	using namespace std;


	Model::Model()
	{
	}

	Model::Model( std::string &path )
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if( !tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, path.c_str(), materialsDir ) )
		{
			throw std::runtime_error( warn + err );
		}

		for( const auto& shape : shapes )
		{
			_meshes.emplace_back( std::make_shared<Mesh>( shape, attrib ) );
		}
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
			//Mesh->DrawVerticesWithImposedShader(); //TODO
		}
	}

	//sets internal model transformation matrix
	void Model::SetModelMatrix(const glm::mat4 & matrix)
	{
		_modelMatrix = matrix;
	}

	//sets model matrix in all of meshes shaders
	void Model::SetModelMatrixUniform()
	{
		/*for (auto &shader : _shaders)
		{
			shader->Activate();
			shader->SetUniformsValue(Uniform<glm::mat4>{ "M", _modelMatrix });
			shader->Deactivate();
		}*/
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
