#ifdef HERMES_PLATFORM_WINDOWS

#include <Windows.h>

#include "Platform/GenericPlatform/PlatformTime.h"

namespace Hermes
{
	static_assert(sizeof(PlatformTimestamp) >= sizeof(LARGE_INTEGER));
	
	namespace WindowsTimeInternal
	{
		LARGE_INTEGER GFrequency;
	}

	void PlatformTime::Init()
	{
		HERMES_ASSERT(QueryPerformanceFrequency(&WindowsTimeInternal::GFrequency));
	}

	PlatformDateTime PlatformTime::GetPlatformTime()
	{
		SYSTEMTIME WinTime;
		PlatformDateTime Result = {};

		GetSystemTime(&WinTime);
		Result.Year = WinTime.wYear;
		Result.Month = WinTime.wMonth;
		Result.Day = WinTime.wDay;
		Result.Hour = WinTime.wHour;
		Result.Minute = WinTime.wMinute;
		Result.Second = WinTime.wSecond;
		Result.Millisecond = WinTime.wMilliseconds;

		return Result;
	}

	PlatformTimestamp PlatformTime::GetCurrentTimestamp()
	{
		LARGE_INTEGER Time = {};
		HERMES_ASSERT(QueryPerformanceCounter(&Time));

		return reinterpret_cast<PlatformTimestamp>(Time.QuadPart);
	}

	float PlatformTime::ToSeconds(const PlatformTimespan& Span)
	{
		auto Start = reinterpret_cast<LONGLONG>(Span.Start);
		auto End = reinterpret_cast<LONGLONG>(Span.End);
		return static_cast<float>(End - Start) / static_cast<float>(WindowsTimeInternal::GFrequency.QuadPart);
	}
}

#endif // HERMES_PLATFORM_WINDOWS
