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
		uint16 Milisecond;
	};

	struct HERMES_API PlatformTime
	{
		static PlatformDateTime GetPlatformTime();
	};
}
