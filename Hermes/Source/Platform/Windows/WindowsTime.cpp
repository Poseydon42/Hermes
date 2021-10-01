#ifdef HERMES_PLATFORM_WINDOWS

#include "Platform/Windows/WindowsTime.h"
#include "Platform/GenericPlatform/PlatformTime.h"

namespace Hermes
{
	namespace WindowsTimeInternal
	{
		LARGE_INTEGER GFrequency;
	}

	PlatformTimespan operator-(const PlatformTimestamp& Lhs, const PlatformTimestamp& Rhs)
	{
		PlatformTimespan Result;
		Result.Start = Rhs;
		Result.End = Lhs;
		return Result;
	}

	void PlatformTime::Init()
	{
		HERMES_ASSERT(QueryPerformanceFrequency(&WindowsTimeInternal::GFrequency));
	}

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

	PlatformTimestamp PlatformTime::GetCurrentTimestamp()
	{
		PlatformTimestamp Result;
		HERMES_ASSERT(QueryPerformanceCounter(&Result.Value));
		return Result;
	}

	float PlatformTime::ToSeconds(const PlatformTimespan& Span)
	{
		return static_cast<float>(Span.End.Value.QuadPart - Span.Start.Value.QuadPart) / static_cast<float>(WindowsTimeInternal::GFrequency.QuadPart);
	}
}

#endif // HERMES_PLATFORM_WINDOWS
