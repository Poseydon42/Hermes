#pragma once

#include "Core/Core.h"
#include "Platform/GenericPlatform/PlatformTime.h"

namespace Hermes
{
	class HERMES_API Timer
	{
	public:
		Timer();

		void Reset();
		
		float GetElapsedTime() const;

	private:
		PlatformTimestamp StartTime;
	};
}