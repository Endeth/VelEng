#include "VMesh.h"
#include <iostream>

namespace Vel
{
	using namespace std;
	VMesh::VMesh()
	{
		_primitive = GL_TRIANGLES;
	}

	VMesh::~VMesh()
	{
		DeleteVertices();
	}

	//loads mesh from file into GPU
	void VMesh::LoadMesh(const char * filename) //TODO loading an actual mesh, test cube for now
	{
		if (_isLoaded)
			return;
		_vertices.reserve(8);
		_vertices.push_back(Vertex(glm::vec3{ 1.0f, 1.0f, 1.0f }, glm::vec3{ 1.0f, 1.0f, 1.0f }, glm::vec2{ 1.0f, 1.0f }));
		_vertices.push_back(Vertex(glm::vec3{ -1.0f, 1.0f, 1.0f }, glm::vec3{ -1.0f, 1.0f, 1.0f }, glm::vec2{ 0.0f, 1.0f }));
		_vertices.push_back(Vertex(glm::vec3{ 1.0f, 1.0f, -1.0f }, glm::vec3{ 1.0f, 1.0f, -1.0f }, glm::vec2{ 1.0f, 0.0f }));
		_vertices.push_back(Vertex(glm::vec3{ -1.0f, 1.0f, -1.0f }, glm::vec3{ -1.0f, 1.0f, -1.0f }, glm::vec2{ 0.0f, 0.0f }));

		_vertices.push_back(Vertex(glm::vec3{ 1.0f, -1.0f, 1.0f }, glm::vec3{ 1.0f, -1.0f, 1.0f }, glm::vec2{ 1.0f, -1.0f }));
		_vertices.push_back(Vertex(glm::vec3{ -1.0f, -1.0f, 1.0f }, glm::vec3{ -1.0f, -1.0f, 1.0f }, glm::vec2{ -1.0f, -1.0f }));
		_vertices.push_back(Vertex(glm::vec3{ 1.0f, -1.0f, -1.0f }, glm::vec3{ 1.0f, -1.0f, -1.0f }, glm::vec2{ 1.0f, 1.0f }));
		_vertices.push_back(Vertex(glm::vec3{ -1.0f, -1.0f, -1.0f }, glm::vec3{ -1.0f, -1.0f, -1.0f }, glm::vec2{ -1.0f, 1.0f }));

		_indices.reserve(36);
		_indices.push_back(0); _indices.push_back(1); _indices.push_back(2);
		_indices.push_back(1); _indices.push_back(2); _indices.push_back(3);

		_indices.push_back(3); _indices.push_back(1); _indices.push_back(7);
		_indices.push_back(1); _indices.push_back(7); _indices.push_back(5);

		_indices.push_back(3); _indices.push_back(2); _indices.push_back(7);
		_indices.push_back(6); _indices.push_back(2); _indices.push_back(7);

		_indices.push_back(0); _indices.push_back(2); _indices.push_back(6);
		_indices.push_back(4); _indices.push_back(6); _indices.push_back(0);

		_indices.push_back(7); _indices.push_back(6); _indices.push_back(5);
		_indices.push_back(6); _indices.push_back(5); _indices.push_back(4);

		_indices.push_back(1); _indices.push_back(0); _indices.push_back(5);
		_indices.push_back(0); _indices.push_back(5); _indices.push_back(4);

		LoadIntoGPU();
		_isLoaded = true;
	}

	void VMesh::LoadVerticesOnly() //DEBUG
	{
			_vertices.reserve(36);
			// Positions                      // Normals                       // Texture Coords
			_vertices.push_back(Vertex(glm::vec3{ -0.5f, -0.5f, -0.5f},  glm::vec3{ 0.0f,  0.0f, -1.0f},  glm::vec2{ 0.0f,  0.0f}));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f, -0.5f, -0.5f},  glm::vec3{ 0.0f,  0.0f, -1.0f},  glm::vec2{ 1.0f,  0.0f}));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f,  0.5f, -0.5f},  glm::vec3{ 0.0f,  0.0f, -1.0f},  glm::vec2{ 1.0f,  1.0f}));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f,  0.5f, -0.5f},  glm::vec3{ 0.0f,  0.0f, -1.0f},  glm::vec2{ 1.0f,  1.0f}));
			_vertices.push_back(Vertex(glm::vec3{ -0.5f,  0.5f, -0.5f}, glm::vec3{ 0.0f,  0.0f, -1.0f},  glm::vec2{ 0.0f,  1.0f}));
			_vertices.push_back(Vertex(glm::vec3{ -0.5f, -0.5f, -0.5f},  glm::vec3{ 0.0f,  0.0f, -1.0f},  glm::vec2{ 0.0f,  0.0f}));

			_vertices.push_back(Vertex(glm::vec3{ -0.5f, -0.5f,  0.5f }, glm::vec3{ 0.0f,  0.0f,  1.0f }, glm::vec2{ 0.0f,  0.0f }));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f, -0.5f,  0.5f},  glm::vec3{ 0.0f,  0.0f,  1.0f },   glm::vec2{ 1.0f,  0.0f }));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f,  0.5f,  0.5f },  glm::vec3{ 0.0f,  0.0f,  1.0f },  glm::vec2{ 1.0f,  1.0f }));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f,  0.5f,  0.5f },  glm::vec3{ 0.0f,  0.0f,  1.0f },   glm::vec2{ 1.0f,  1.0f }));
			_vertices.push_back(Vertex(glm::vec3{ -0.5f,  0.5f,  0.5f },  glm::vec3{ 0.0f,  0.0f,  1.0f },  glm::vec2{ 0.0f,  1.0f }));
			_vertices.push_back(Vertex(glm::vec3{ -0.5f, -0.5f,  0.5f },  glm::vec3{ 0.0f,  0.0f,  1.0f },  glm::vec2{ 0.0f,  0.0f }));

			_vertices.push_back(Vertex(glm::vec3{ -0.5f,  0.5f,  0.5f }, glm::vec3{ -1.0f,  0.0f,  0.0f },  glm::vec2{ 1.0f,  0.0f }));
			_vertices.push_back(Vertex(glm::vec3{ -0.5f,  0.5f, -0.5f }, glm::vec3{ -1.0f,  0.0f,  0.0f },  glm::vec2{ 1.0f,  1.0f }));
			_vertices.push_back(Vertex(glm::vec3{ -0.5f, -0.5f, -0.5f }, glm::vec3{ -1.0f,  0.0f,  0.0f },  glm::vec2{ 0.0f,  1.0f }));
			_vertices.push_back(Vertex(glm::vec3{ -0.5f, -0.5f, -0.5f }, glm::vec3{ -1.0f,  0.0f,  0.0f },  glm::vec2{ 0.0f,  1.0f }));
			_vertices.push_back(Vertex(glm::vec3{ -0.5f, -0.5f,  0.5f }, glm::vec3{ -1.0f,  0.0f,  0.0f },  glm::vec2{ 0.0f,  0.0f }));
			_vertices.push_back(Vertex(glm::vec3{ -0.5f,  0.5f,  0.5f }, glm::vec3{ -1.0f,  0.0f,  0.0f },  glm::vec2{ 1.0f,  0.0f }));

			_vertices.push_back(Vertex(glm::vec3{ 0.5f,  0.5f,  0.5f },  glm::vec3{ 1.0f,  0.0f,  0.0f },  glm::vec2{ 1.0f,  0.0f }));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f,  0.5f, -0.5f },  glm::vec3{ 1.0f,  0.0f,  0.0f },  glm::vec2{ 1.0f,  1.0f }));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f, -0.5f, -0.5f },  glm::vec3{ 1.0f,  0.0f,  0.0f },  glm::vec2{ 0.0f,  1.0f }));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f, -0.5f, -0.5f },  glm::vec3{ 1.0f,  0.0f,  0.0f },  glm::vec2{ 0.0f,  1.0f }));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f, -0.5f,  0.5f }, glm::vec3{ 1.0f,  0.0f,  0.0f },  glm::vec2{ 0.0f,  0.0f }));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f,  0.5f,  0.5f },  glm::vec3{ 1.0f,  0.0f,  0.0f },  glm::vec2{ 1.0f,  0.0f }));

			_vertices.push_back(Vertex(glm::vec3{ -0.5f, -0.5f, -0.5f },  glm::vec3{ 0.0f, -1.0f,  0.0f },  glm::vec2{ 0.0f,  1.0f }));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f, -0.5f, -0.5f },  glm::vec3{ 0.0f, -1.0f,  0.0f },   glm::vec2{ 1.0f,  1.0f }));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f, -0.5f,  0.5f },  glm::vec3{ 0.0f, -1.0f,  0.0f },  glm::vec2{ 1.0f,  0.0f }));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f, -0.5f,  0.5f }, glm::vec3{ 0.0f, -1.0f,  0.0f },  glm::vec2{ 1.0f,  0.0f }));
			_vertices.push_back(Vertex(glm::vec3{ -0.5f, -0.5f,  0.5f },  glm::vec3{ 0.0f, -1.0f,  0.0f },  glm::vec2{ 0.0f,  0.0f }));
			_vertices.push_back(Vertex(glm::vec3{ -0.5f, -0.5f, -0.5f }, glm::vec3{ 0.0f, -1.0f,  0.0f },  glm::vec2{ 0.0f,  1.0f }));

			_vertices.push_back(Vertex(glm::vec3{ -0.5f,  0.5f, -0.5f },  glm::vec3{ 0.0f,  1.0f,  0.0f },  glm::vec2{ 0.0f,  1.0f }));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f,  0.5f, -0.5f },  glm::vec3{ 0.0f,  1.0f,  0.0f },  glm::vec2{ 1.0f,  1.0f }));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f,  0.5f,  0.5f },  glm::vec3{ 0.0f,  1.0f,  0.0f },   glm::vec2{ 1.0f,  0.0f }));
			_vertices.push_back(Vertex(glm::vec3{ 0.5f,  0.5f,  0.5f },  glm::vec3{ 0.0f,  1.0f,  0.0f },  glm::vec2{ 1.0f,  0.0f }));
			_vertices.push_back(Vertex(glm::vec3{ -0.5f,  0.5f,  0.5f }, glm::vec3{ 0.0f,  1.0f,  0.0f },  glm::vec2{ 0.0f,  0.0f }));
			_vertices.push_back(Vertex(glm::vec3{ -0.5f,  0.5f, -0.5f },  glm::vec3{ 0.0f,  1.0f,  0.0f },   glm::vec2{ 0.0f,  1.0f }));
	


		_vboVertices.FillBuffer(_vertices.size(), &_vertices[0]);
	}

	void VMesh::SetMaterial(const std::shared_ptr<VMaterial>& mat)
	{
		_material = mat;
	}

	void VMesh::DeleteVertices()
	{
	}
	// fills both vertices and indices buffers
	void VMesh::LoadIntoGPU()
	{
		_vboVertices.FillBuffer(_vertices.size(), &_vertices[0]);
		_vboIndices.FillBuffer(_indices.size(), &_indices[0]);
	}

	void VMesh::UpdateVerticesInGPU()
	{
	}

	void VMesh::SetVerticesVAO()
	{
		auto stride = sizeof(Vertex);

		glBindVertexArray(_vaoID);

		_vboVertices.BindBuffer();

		glEnableVertexAttribArray(_shader->GetAttribute("vVertex"));
		glVertexAttribPointer(_shader->GetAttribute("vVertex"), 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offsetof(Vertex, position));

		glEnableVertexAttribArray(_shader->GetAttribute("vNormal"));
		glVertexAttribPointer(_shader->GetAttribute("vNormal"), 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offsetof(Vertex, normal));

		glEnableVertexAttribArray(_shader->GetAttribute("vUV"));
		glVertexAttribPointer(_shader->GetAttribute("vUV"), 2, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offsetof(Vertex, UV));

		_vboIndices.BindBuffer();

		glBindVertexArray(0);
		_vboIndices.UnbindBuffer();
		_vboVertices.UnbindBuffer();
	}

	void VMesh::SetVAO()
	{
		auto stride = sizeof(Vertex);

		glBindVertexArray(_vaoID);

		_vboVertices.BindBuffer();

		glEnableVertexAttribArray(_shader->GetAttribute("vVertex"));
		glVertexAttribPointer(_shader->GetAttribute("vVertex"), 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offsetof(Vertex, position));

		glEnableVertexAttribArray(_shader->GetAttribute("vNormal"));
		glVertexAttribPointer(_shader->GetAttribute("vNormal"), 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offsetof(Vertex, normal));

		glEnableVertexAttribArray(_shader->GetAttribute("vUV"));
		glVertexAttribPointer(_shader->GetAttribute("vUV"), 2, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offsetof(Vertex, UV));

		glBindVertexArray(0);
	}
	void VMesh::BindAdditionalDrawingOptions()
	{
		_material->BindMaterial();
		/*if (_material != nullptr)
		{
			_shader->Activate();
			_shader->SetUniformsValue(Uniform<glm::vec3>{ "material.ambient", _material->_ambient});
			_shader->SetUniformsValue(Uniform<glm::vec3>{ "material.diffuse", _material->_diffuse});
			_shader->SetUniformsValue(Uniform<glm::vec3>{ "material.specular", _material->_specular});
			_shader->SetUniformsValue(Uniform<GLfloat>{ "material.shininess", _material->_shininess});
			_shader->Deactivate();
		}*/
	}

	void VMesh::UnbindAdditionalDrawingOptions()
	{
		_material->UnbindMaterial();
	}

	VPlaneMesh::VPlaneMesh()
	{
		LoadVerticesOnly();
	}

	void VPlaneMesh::LoadMesh(const char * filename)
	{
		if (_isLoaded)
			return;
		_vertices.reserve(8);
		_vertices.push_back(Vertex(glm::vec3{ 1.0f, 0.0f, 1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 1.0f, 1.0f }));
		_vertices.push_back(Vertex(glm::vec3{ -1.0f, 0.0f, 1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 0.0f, 1.0f }));
		_vertices.push_back(Vertex(glm::vec3{ 1.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 1.0f, 0.0f }));
		_vertices.push_back(Vertex(glm::vec3{ -1.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 0.0f, 0.0f }));

		_indices.reserve(6);
		_indices.push_back(0); _indices.push_back(1); _indices.push_back(2);
		_indices.push_back(1); _indices.push_back(2); _indices.push_back(3);

		LoadIntoGPU();
		_isLoaded = true;
	}
	void VPlaneMesh::LoadVerticesOnly()
	{
		_vertices.reserve(6);
		_vertices.push_back(Vertex(glm::vec3{ 1.0f, 0.0f, 1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 1.0f, 1.0f }));
		_vertices.push_back(Vertex(glm::vec3{ -1.0f, 0.0f, 1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 0.0f, 1.0f }));
		_vertices.push_back(Vertex(glm::vec3{ 1.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 1.0f, 0.0f }));

		_vertices.push_back(Vertex(glm::vec3{ -1.0f, 0.0f, 1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 0.0f, 1.0f }));
		_vertices.push_back(Vertex(glm::vec3{ 1.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 1.0f, 0.0f }));
		_vertices.push_back(Vertex(glm::vec3{ -1.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 0.0f, 0.0f }));

		_vboVertices.FillBuffer(_vertices.size(), &_vertices[0]);
	}
}
