#pragma once
#include <chrono>
#include "VSignal.h"

namespace Vel
{
	/*class VFrameClock
	{
	public:   
		void Tick();
		const double GetTime() const { return currentTime; }
		const double GetFrameRate() const { return frameRate; }
		const double GetTimeDifferance() const { return currentTime - prevTime; }
	private:
		double prevTime{ 0.0 };
		double currentTime{ 0.0 };
		double frameRate{1.0/60.0};
	};*/
	class SignalClock
	{
	public:
		SignalClock();

		Signal<void(float)> OnTick;
		void UpdateTime();

	private:
		

		std::chrono::system_clock::time_point _clockStart;
		//float currentTime;


	};

	class Timer
	{
	public:
		Timer() : _beg(Clock::now()) {}
		void Reset() { _beg = Clock::now(); }
		double Elapsed() const
		{
			return std::chrono::duration_cast<Second>(Clock::now() - _beg).count();
		}
	private:
		using Clock = std::chrono::high_resolution_clock;
		using Second = std::chrono::duration<double, std::ratio<1>>;
		std::chrono::time_point<Clock> _beg;
	};
}