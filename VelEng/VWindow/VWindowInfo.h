#pragma once
#include <string>

#include "../VOpenGL/glew.h"
#include "../VOpenGL/glfw3.h"
#include "../VOpenGL/glm/glm.hpp"

class WindowInfo
{
public:
	WindowInfo() = delete;
	WindowInfo(const std::string& Name, const glm::uvec2 &Position, const  glm::uvec2 &Size, bool IsResizeable = false);
	WindowInfo(const std::string& Name, int PosX, int PosY, int Width, int Height, bool IsResizeable = false);

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