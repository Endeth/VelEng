#pragma once

#include <functional>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include "external/glm/gtx/hash.hpp"
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

	struct VertexUVColor
	{
		VertexUVColor() : position( 0 ), color( 0 ), UV( 0 )
		{
		}
		VertexUVColor( const glm::vec3 &pos, const glm::vec4 &color, const glm::vec2 &uvCoor = glm::vec2{ 0.0f, 0.0f } ) : position( pos ), color( color ), UV( uvCoor )
		{
		}

		bool operator==( const VertexUVColor& other ) const
		{
			return position == other.position && color == other.color && UV == other.UV;
		}

		glm::vec3 position;
		glm::vec4 color;
		glm::vec2 UV;
	};


}

template<>
struct std::hash<Vel::VertexUVColor>
{
	size_t operator()( Vel::VertexUVColor const& vertex ) const
	{
		return ( ( hash<glm::vec3>()( vertex.position ) ^ ( hash<glm::vec3>()( vertex.color ) << 1 ) ) >> 1 ) ^ ( hash<glm::vec2>()( vertex.UV ) << 1 );
	}
};