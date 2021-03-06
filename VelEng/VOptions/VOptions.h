#pragma once
#include "../VOpenGL/glew.h"
#include "../VOpenGL/glm/glm.hpp"

namespace Vel
{
	class Options
	{
	public:
		const glm::vec2& GetResolution();
		const GLfloat GetMaxFrameRate();
	private:
		glm::vec2 _resolution;
		GLfloat _maxFrameRate;
	};

}