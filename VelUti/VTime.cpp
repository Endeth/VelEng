
#include "VTime.h"

namespace Vel
{
	using namespace std;

	SignalClock::SignalClock()
	{
		_clockStart;
	}

	void SignalClock::UpdateTime()
	{
		OnTick(1.0f);
	}
	/*void VFrameClock::Tick()
	{
		_prevTime = _currentTime;
		_currentTime += _frameRate;
	}*/
}