#pragma once

#include "Core/Core.h"

namespace Hermes
{
	struct PlatformDateTime
	{
		uint16 Year;
		uint16 Month;
		uint16 Day;
		uint16 Hour;
		uint16 Minute;
		uint16 Second;
		uint16 Millisecond;
	};
	
	/*
	 * Opaque struct that represent a point on a timeline
	 * It has to have maximum possible accuracy
	 */
	using PlatformTimestamp = void*;

	/*
	 * Opaque struct that represents a duration of time between two PlatformTimestamps
	 */
	struct PlatformTimespan
	{
		PlatformTimestamp Start;
		PlatformTimestamp End;
	};

	struct HERMES_API PlatformTime
	{
		static void Init();

		static PlatformDateTime GetPlatformTime();

		static PlatformTimestamp GetCurrentTimestamp();

		static float ToSeconds(const PlatformTimespan& Span);
	};	
}
