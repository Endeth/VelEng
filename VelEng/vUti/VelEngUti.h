#pragma once

#include <vector>
#include <thread>
#include <chrono>

#include "external/glfw/glfw3.h"
#include "external/glm/glm.hpp"

namespace Vel
{
	class FrameClock
	{
	public:
		void Tick();
		const float GetTime() const { return currentTime; }
		const float GetSimulationTime() const { return tick; }
		const float GetFrameRate() const { return frameRate; }
		const float GetTimeDifferance() const { return currentTime - prevTime; }
		void CapFPS();
	private:
		const float CalculateRenderTime();

		float prevTime{ 0.0 };
		float tick{ 0.0 };
		float currentTime{ 0.0 };
		float frameRate{ 1.0f / 60.0f };
	};
}