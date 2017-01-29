#pragma once
#include <chrono>
#include "VelSignal.h"

namespace Vel
{
	/*class VFrameClock
	{
	public:   
		void Tick();
		const double GetTime() const { return _currentTime; }
		const double GetFrameRate() const { return _frameRate; }
		const double GetTimeDifferance() const { return _currentTime - _prevTime; }
	private:
		double _prevTime{ 0.0 };
		double _currentTime{ 0.0 };
		double _frameRate{1.0/60.0};
	};*/
	class VSignalClock
	{
	public:
		VSignalClock();

		Signal<void(float)> OnTick;
		void UpdateTime();

	private:
		

		std::chrono::system_clock::time_point _clockStart;
		//float _currentTime;


	};

	class VTimer
	{
	public:
		VTimer() : _beg(Clock::now()) {}
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