#pragma once

#include "external/glm/glm.hpp"

namespace Vel
{
	class Options
	{
	public:
		const glm::vec2& GetResolution();
		const float GetMaxFrameRate();
	private:
		glm::vec2 _resolution;
		float _maxFrameRate;
	};

}