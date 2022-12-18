#pragma once

#ifdef HERMES_PLATFORM_WINDOWS

#include <Windows.h>

namespace Hermes
{
	struct PlatformTimestamp
	{
		LARGE_INTEGER Value;
	};

	struct PlatformTimespan
	{
		PlatformTimestamp Start;
		PlatformTimestamp End;
	};

	namespace WindowsTimeInternal
	{
		extern LARGE_INTEGER GFrequency;
	}
}

#endif
