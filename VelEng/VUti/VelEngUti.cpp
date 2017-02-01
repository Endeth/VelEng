
#include "VelEngUti.h"
#include <algorithm>


Vel::VMouse::VMouse(int x, int y) : _mouseReset(true)
{
	_oldPosition = glm::ivec2{ x, y };
	_currentPosition = glm::ivec2{ x, y };
}

const bool Vel::VMouse::ShouldReset(const glm::ivec2 & min, const glm::ivec2 & max) const
{
	bool withinBoundaries = ((_currentPosition.x < min.x || _currentPosition.y < min.y) || ((_currentPosition.x > max.x || _currentPosition.y > max.y)));
	return (withinBoundaries && _mouseReset);
}

void Vel::VMouse::ResetIfOutside(GLFWwindow* window, const glm::ivec2 & min, const glm::ivec2 & max)
{
	if (ShouldReset(min, max))
	{
		ResetMousePosition(window, glm::ivec2((min.x + max.x) /2, (min.y + max.y)/2));
	}
}

void Vel::VMouse::SetCurrentPosition(int x, int y)
{
	_oldPosition = _currentPosition;
	_currentPosition = glm::ivec2{ x, y };
	_diff += glm::ivec2(_oldPosition.x - _currentPosition.x, _currentPosition.y - _oldPosition.y);
}

void Vel::VMouse::SetCurrentPosition(const glm::ivec2 & newPosition)
{
	_oldPosition = _currentPosition;
	_currentPosition = newPosition;
}

void Vel::VMouse::ResetMousePosition(GLFWwindow* window, const glm::ivec2 & pos)
{
	_oldPosition = pos;
	_currentPosition = pos;
	glfwSetCursorPos(window, pos.x, pos.y);
	_diff += glm::ivec2{ 0,0 };
}

void Vel::VKeyboard::KeyPress(const int key)
{
	if (_pressedKeys.size() < _maxPressedKeys)
	{
		_pressedKeys.push_back(key);
	}
}

void Vel::VKeyboard::KeyUp(const int key)
{
	_pressedKeys.erase(std::remove(_pressedKeys.begin(), _pressedKeys.end(), key), _pressedKeys.end());
}

void Vel::VKeyboard::KeyHandler(int key, int action, int mods)
{
	if (action == GLFW_PRESS)
		KeyPress(key);
	else if (action == GLFW_RELEASE)
		KeyUp(key);
}

const bool Vel::VKeyboard::IsKeyPressed(const int key) const
{
	return (std::find(_pressedKeys.begin(), _pressedKeys.end(), key) != _pressedKeys.end());
}

void Vel::VFrameClock::Tick()
{
	_prevTime = _currentTime;
	_currentTime = glfwGetTime();
	_tick += _frameRate;
}

const double Vel::VFrameClock::CalculateRenderTime()
{
	_prevTime = _currentTime;
	_currentTime = glfwGetTime();
	return _currentTime - _prevTime;
}

void Vel::VFrameClock::CapFPS()
{
	unsigned int sleepTime = (_frameRate - CalculateRenderTime()) * 1000;
	if (sleepTime > 0)
		std::this_thread::sleep_for(std::chrono::microseconds());
}
 