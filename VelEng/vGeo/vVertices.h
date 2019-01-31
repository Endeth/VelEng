#pragma once

#include "external/glm/glm.hpp"

namespace Vel
{
	struct Vertex
	{
		Vertex( const glm::vec3 &pos, const glm::vec3 &norm, const glm::vec2 &uvCoor = glm::vec2{ 0.0f, 0.0f } ) : position( pos ), normal( norm ), UV( uvCoor )
		{
		}
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 UV;
	};

	struct VertexUV
	{
		VertexUV( const glm::vec3 &pos, const glm::vec2 &uvCoor = glm::vec2{ 0.0f, 0.0f } ) : position( pos ), UV( uvCoor )
		{
		}
		glm::vec3 position;
		glm::vec2 UV;
	};

	struct VertexColor
	{
		VertexColor( const glm::vec3 &pos, const glm::vec4 &color ) : position( pos ), color( color )
		{
		}
		glm::vec3 position;
		glm::vec4 color;
	};
}