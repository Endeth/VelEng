#include <algorithm>

#include "VelEngUti.h"

void Vel::FrameClock::Tick()
{
	prevTime = currentTime;
	currentTime = glfwGetTime();
	tick += frameRate;
}

const float Vel::FrameClock::CalculateRenderTime()
{
	prevTime = currentTime;
	currentTime = glfwGetTime();
	return currentTime - prevTime;
}

void Vel::FrameClock::CapFPS()
{
	unsigned int sleepTime = (frameRate - CalculateRenderTime()) * 1000;
	if (sleepTime > 0)
		std::this_thread::sleep_for(std::chrono::microseconds());
}
 