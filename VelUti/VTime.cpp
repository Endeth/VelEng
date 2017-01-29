
#include "VTime.h"

namespace Vel
{
	using namespace std;

	VSignalClock::VSignalClock()
	{
		_clockStart;
	}

	void VSignalClock::UpdateTime()
	{
		OnTick(1.0f);
	}
	/*void VFrameClock::Tick()
	{
		_prevTime = _currentTime;
		_currentTime += _frameRate;
	}*/
}