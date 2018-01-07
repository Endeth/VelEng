#pragma once

#include <string>

#include "external/glfw/glfw3.h"
#include "external/glm/glm.hpp"


namespace Vel
{
    class WindowInfo
    {
    public:
        WindowInfo() = delete;
        WindowInfo( const std::string& Name, const glm::uvec2 &Position, const  glm::uvec2 &Size, bool IsResizeable = false ) {}
        WindowInfo( const std::string& Name, int PosX, int PosY, int Width, int Height, bool IsResizeable = false ) {}

        const glm::uvec2& GetPosition() const { return _position; }
        const glm::uvec2& GetSize() const { return _size; }
        std::string GetName() const { return _name; }
        bool IsResizeable() const { return _isResizeable; }
    private:
        std::string _name;
        glm::uvec2 _size;
        glm::uvec2 _position;
        bool _isResizeable;
    };

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