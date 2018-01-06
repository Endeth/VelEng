#pragma once

#include "external/glfw/glfw3.h"
#include "external/glm/glm.hpp"
#include "vWindowInfo.h"

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