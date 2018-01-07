#include "vPlane.h"

namespace Vel
{
	const float EPSILON = 0.0001f;

	VPlane::VPlane()
	{
	}

	VPlane::VPlane(const glm::vec3 & Normal, const glm::vec3 & Pos)
	{
		this->Normal = Normal;
		Position = Pos;
		D = -glm::dot(Normal, Pos);
	}

	VPlane::~VPlane()
	{
	}

	VPlane VPlane::FromPoints(const glm::vec3 & V1, const glm::vec3 & V2, const glm::vec3 & V3)
	{
		VPlane temp;
		glm::vec3 e1 = V2 - V1;
		glm::vec3 e2 = V3 - V1;
		temp.Normal = glm::normalize(glm::cross(e1, e2));
		temp.D = -glm::dot(temp.Normal, V1);
		return temp;
	}

	float VPlane::GetDistance(const glm::vec3 & P)
	{
		return glm::dot(Normal, P) + D;
	}
}