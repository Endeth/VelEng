#pragma once
namespace Vel
{
	struct Line
	{
		Line(const glm::vec3 &Pos1, const glm::vec3 &Pos2) : _points{ Pos1, Pos2 }
		{
		}
		const glm::vec3 _points[2];
	};
}