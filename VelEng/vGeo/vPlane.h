#pragma once

#include "external/glm/glm.hpp"

namespace Vel
{
	struct VPlane
	{
		VPlane();
		VPlane(const glm::vec3 &Normal, const glm::vec3 &Pos);
		~VPlane();

		static VPlane FromPoints(const glm::vec3 &V1, const glm::vec3 &V2, const glm::vec3 &V3);
		float GetDistance(const glm::vec3 &P);

		glm::vec3 Position;
		glm::vec3 Normal;
		float D;
	};
}