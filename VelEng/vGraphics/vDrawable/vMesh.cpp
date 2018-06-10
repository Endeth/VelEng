#include <iostream>

#include "vMesh.h"

namespace Vel
{
	using namespace std;
	Mesh::Mesh()
	{
		//_primitive = GL_TRIANGLES;
	}

	Mesh::~Mesh()
	{
		DeleteVertices();
	}

	//loads mesh from file into GPU
	void Mesh::LoadMesh(const char * filename) //TODO loading an actual mesh, test cube for now
	{
		if (_isLoaded)
			return;

		LoadIntoGPU();
		_isLoaded = true;
	}

	void Mesh::LoadVerticesOnly() //DEBUG
	{
		//_vboVertices.FillBuffer(_vertices.size(), &_vertices[0]);
	}

	void Mesh::SetMaterial(const std::shared_ptr<Material>& mat)
	{
		_material = mat;
	}

	void Mesh::DeleteVertices()
	{
	}
	// fills both vertices and indices buffers
	void Mesh::LoadIntoGPU()
	{
		_vboVertices.FillBuffer(_vertices.size(), &_vertices[0]);
		_vboIndices.FillBuffer(_indices.size(), &_indices[0]);
	}

	void Mesh::UpdateVerticesInGPU()
	{
	}

	void Mesh::SetVerticesVAO()
	{
		/*auto stride = sizeof(Vertex);

		glBindVertexArray(_vaoID);

		_vboVertices.BindBuffer();

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offsetof(Vertex, position));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offsetof(Vertex, normal));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offsetof(Vertex, UV));

		_vboIndices.BindBuffer();

		glBindVertexArray(0);
		_vboIndices.UnbindBuffer();
		_vboVertices.UnbindBuffer();*/
	}

	void Mesh::SetVAO()
	{
		/*auto stride = sizeof(Vertex);

		glBindVertexArray(_vaoID);

		_vboVertices.BindBuffer();

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offsetof(Vertex, position));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offsetof(Vertex, normal));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offsetof(Vertex, UV));

		glBindVertexArray(0);*/
	}
	void Mesh::BindAdditionalDrawingOptions()
	{
		_material->BindMaterial();
	}

	void Mesh::UnbindAdditionalDrawingOptions()
	{
		_material->UnbindMaterial();
	}

	PlaneMesh::PlaneMesh()
	{
		LoadVerticesOnly();
	}

	void PlaneMesh::LoadMesh(const char * filename)
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
		_indices.push_back(1); _indices.push_back(3); _indices.push_back(2);

		LoadIntoGPU();
		_isLoaded = true;
	}
	void PlaneMesh::LoadVerticesOnly()
	{
		_vertices.reserve(6);
		_vertices.push_back(Vertex(glm::vec3{ 1.0f, 0.0f, 1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 1.0f, 1.0f }));
		_vertices.push_back(Vertex(glm::vec3{ 1.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 1.0f, 0.0f }));
		_vertices.push_back(Vertex(glm::vec3{ -1.0f, 0.0f, 1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 0.0f, 1.0f }));

		_vertices.push_back(Vertex(glm::vec3{ -1.0f, 0.0f, 1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 0.0f, 1.0f }));
		_vertices.push_back(Vertex(glm::vec3{ 1.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 1.0f, 0.0f }));
		_vertices.push_back(Vertex(glm::vec3{ -1.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 0.0f, 0.0f }));

		_vboVertices.FillBuffer(_vertices.size(), &_vertices[0]);
	}
}
