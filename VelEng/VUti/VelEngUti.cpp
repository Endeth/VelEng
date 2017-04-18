
#include "VelEngUti.h"
#include <algorithm>


Vel::Mouse::Mouse(int x, int y) : _mouseReset(false)
{
	_oldPosition = glm::ivec2{ x, y };
	_currentPosition = glm::ivec2{ x, y };
}

const bool Vel::Mouse::ShouldReset(const glm::ivec2 & min, const glm::ivec2 & max) const
{
	bool withinBoundaries = ((_currentPosition.x < min.x || _currentPosition.y < min.y) || ((_currentPosition.x > max.x || _currentPosition.y > max.y)));
	return (withinBoundaries && _mouseReset);
}

void Vel::Mouse::ResetIfOutside(GLFWwindow* window, const glm::ivec2 & min, const glm::ivec2 & max)
{
	if (ShouldReset(min, max))
	{
		ResetMousePosition(window, glm::ivec2((min.x + max.x) /2, (min.y + max.y)/2));
	}
}

void Vel::Mouse::SetCurrentPosition(int x, int y)
{
	_oldPosition = _currentPosition;
	_currentPosition = glm::ivec2{ x, y };
	_diff += glm::ivec2(_oldPosition.x - _currentPosition.x, _currentPosition.y - _oldPosition.y);
}

void Vel::Mouse::SetCurrentPosition(const glm::ivec2 & newPosition)
{
	_oldPosition = _currentPosition;
	_currentPosition = newPosition;
}

void Vel::Mouse::ResetMousePosition(GLFWwindow* window, const glm::ivec2 & pos)
{
	_oldPosition = pos;
	_currentPosition = pos;
	glfwSetCursorPos(window, pos.x, pos.y);
	_diff += glm::ivec2{ 0,0 };
}

void Vel::Keyboard::KeyPress(const int key)
{
	if (_pressedKeys.size() < _maxPressedKeys)
	{
		_pressedKeys.push_back(key);
	}
}

void Vel::Keyboard::KeyUp(const int key)
{
	_pressedKeys.erase(std::remove(_pressedKeys.begin(), _pressedKeys.end(), key), _pressedKeys.end());
}

void Vel::Keyboard::KeyHandler(int key, int action, int mods)
{
	if (action == GLFW_PRESS)
		KeyPress(key);
	else if (action == GLFW_RELEASE)
		KeyUp(key);
}

const bool Vel::Keyboard::IsKeyPressed(const int key) const
{
	return (std::find(_pressedKeys.begin(), _pressedKeys.end(), key) != _pressedKeys.end());
}

void Vel::FrameClock::Tick()
{
	_prevTime = _currentTime;
	_currentTime = glfwGetTime();
	_tick += _frameRate;
}

const GLfloat Vel::FrameClock::CalculateRenderTime()
{
	_prevTime = _currentTime;
	_currentTime = glfwGetTime();
	return _currentTime - _prevTime;
}

void Vel::FrameClock::CapFPS()
{
	unsigned int sleepTime = (_frameRate - CalculateRenderTime()) * 1000;
	if (sleepTime > 0)
		std::this_thread::sleep_for(std::chrono::microseconds());
}
 