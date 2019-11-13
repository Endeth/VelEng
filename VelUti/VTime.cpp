
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
		prevTime = currentTime;
		currentTime += frameRate;
	}*/
}