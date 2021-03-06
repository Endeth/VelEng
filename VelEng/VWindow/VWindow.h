#pragma once

#include "../VOpenGL/glew.h"
#include "../VOpenGL/glfw3.h"
#include "../VOpenGL/glm/glm.hpp"
#include "VWindowInfo.h"

namespace Vel
{
	class Window
	{
	public:
		Window();
		Window(const WindowInfo& info);
		~Window();
		void SetAsContext() { glfwMakeContextCurrent(_window); }
		const glm::uvec2& GetSize() const 
		{ 
			return _info.GetSize(); 
		}
		GLFWwindow* GetGLFWWindow() { return _window; }
	private:
		void InitWindow();

		WindowInfo _info;
		GLFWwindow* _window;
	};
}