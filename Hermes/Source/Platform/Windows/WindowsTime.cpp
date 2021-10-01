#ifdef HERMES_PLATFORM_WINDOWS

#include <Windows.h>

#include "Platform/GenericPlatform/PlatformTime.h"

namespace Hermes
{
	PlatformDateTime PlatformTime::GetPlatformTime()
	{
		SYSTEMTIME WinTime;
		PlatformDateTime Result;

		GetSystemTime(&WinTime);
		Result.Year = WinTime.wYear;
		Result.Month = WinTime.wMonth;
		Result.Day = WinTime.wDay;
		Result.Hour = WinTime.wHour;
		Result.Minute = WinTime.wMinute;
		Result.Second = WinTime.wSecond;
		Result.Milisecond = WinTime.wMilliseconds;

		return Result;
	}
}

#endif // HERMES_PLATFORM_WINDOWS
