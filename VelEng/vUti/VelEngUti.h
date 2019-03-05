#pragma once

#include <vector>
#include <thread>
#include <chrono>

#include "external/glfw/glfw3.h"
#include "external/glm/glm.hpp"

namespace Vel
{
	class Mouse
	{
	public:
		Mouse( int x = 0, int y = 0 );

		const glm::ivec2& GetCurrentPosition() const { return _currentPosition; }
		const glm::ivec2& GetOldPosition() const { return _oldPosition; }
		const glm::ivec2& GetPositionDifference() const { return _diff; }
		const bool IsReseting() const { return _mouseReset; }
		void ResetIfOutside(GLFWwindow* window, const glm::ivec2 &min, const glm::ivec2 &max);

		void ChangeResetting() { _mouseReset = !_mouseReset; }
		void SetCurrentPosition( int x, int y );
		void SetCurrentPosition( const glm::ivec2& newPosition );

		void ResetMousePosition( GLFWwindow* window, const glm::ivec2& pos = glm::ivec2{ 0,0 } );
	protected:
		const bool ShouldReset( const glm::ivec2 &min, const glm::ivec2 &max ) const;

		glm::ivec2 _currentPosition;
		glm::ivec2 _oldPosition;
		glm::ivec2 _diff;

		float _mouseSpeed;
		bool _mouseReset;
	};

	class Keyboard
	{
	public:
		void KeyHandler(int key, int action, int mods);
		const bool IsKeyPressed(const int key) const;
	private:
		void KeyPress(const int key);
		void KeyUp(const int key);
		std::vector<int> _pressedKeys;
		size_t _maxPressedKeys{ 4 };
	};

	class FrameClock
	{
	public:
		void Tick();
		const float GetTime() const { return _currentTime; }
		const float GetSimulationTime() const { return _tick; }
		const float GetFrameRate() const { return _frameRate; }
		const float GetTimeDifferance() const { return _currentTime - _prevTime; }
		void CapFPS();
	private:
		const float CalculateRenderTime();

		float _prevTime{ 0.0 };
		float _tick{ 0.0 };
		float _currentTime{ 0.0 };
		float _frameRate{ 1.0f / 60.0f };
	};
}