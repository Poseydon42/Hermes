#include "Timer.h"

namespace Hermes
{
	Timer::Timer()
		: StartTime(PlatformTime::GetCurrentTimestamp())
	{
	}

	float Timer::GetElapsedTime() const
	{
		auto CurrentTime = PlatformTime::GetCurrentTimestamp();
		return PlatformTime::ToSeconds({ StartTime, CurrentTime });
	}

	void Timer::Reset()
	{
		StartTime = PlatformTime::GetCurrentTimestamp();
	}
}
