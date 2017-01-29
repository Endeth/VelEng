#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "VWindowInfo.h"

namespace Vel
{
	class VWindow
	{
	public:
		VWindow();
		VWindow(const VWindowInfo& info);
		~VWindow();
		void SetAsContext() { glfwMakeContextCurrent(_window); }
		const glm::uvec2& GetSize() const 
		{ 
			return _info.GetSize(); 
		}
		GLFWwindow* GetGLFWWindow() { return _window; }
	private:
		void InitWindow();

		VWindowInfo _info;
		GLFWwindow* _window;
	};
}