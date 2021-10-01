#pragma once

#ifdef HERMES_PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Core/Core.h"

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
