#define TINYOBJLOADER_IMPLEMENTATION
#include <iostream>

#include "vMesh.h"

namespace Vel
{
	using namespace std;

	Mesh::Mesh( const tinyobj::shape_t &shape, const tinyobj::attrib_t &attrib )
	{
		std::unordered_map<VertexUVColor, uint32_t> uniqueVertices = {};

		for( const auto& index : shape.mesh.indices )
		{
			VertexUVColor vertex;
			vertex.position = glm::vec3( attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2] );
			vertex.color = glm::vec4( attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2], 1.f );
			vertex.UV = glm::vec2( attrib.texcoords[2 * index.texcoord_index + 0], 1 - attrib.texcoords[2 * index.texcoord_index + 1] );

			if( uniqueVertices[vertex] == 0 )
			{
				uniqueVertices[vertex] = static_cast<uint32_t>( _vertices.size() );
				_vertices.push_back( vertex );
			}

			_indices.push_back( uniqueVertices[vertex] );
		}
	}

	Mesh::~Mesh()
	{
		//DeleteVertices();
	}

	void Mesh::SetMaterial(const std::shared_ptr<Material>& mat)
	{
		_material = mat;
	}

	void Mesh::LoadIntoGPU()
	{
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
}
