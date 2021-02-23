#pragma once

#include "Core/Core.h"
#include "Core/Log/ILogDevice.h"
#include "Platform/GenericPlatform/PlatformDebug.h"

namespace Hermes
{
	class HERMES_API DebugLogDevice : public ILogDevice
	{
	public:
		inline DebugLogDevice(LogLevel InitialLogLevel = LogLevel::Trace) : CurrentLogLevel(InitialLogLevel) {}

		inline void Write(LogLevel Level, String Text) override
		{
			if (Level >= CurrentLogLevel)
				PlatformDebug::PrintString(Text);
		}

		inline void WriteLine(LogLevel Level, String Text) override
		{
			if (Level >= CurrentLogLevel)
				PlatformDebug::PrintString(Text + L"\n");
		}

		inline LogLevel GetCurrentLogLevel() override
		{
			return CurrentLogLevel;
		}

		inline void SetCurrentLogLevel(LogLevel NewLevel) override
		{
			CurrentLogLevel = NewLevel;
		}

	private:
		LogLevel CurrentLogLevel;
	};
}