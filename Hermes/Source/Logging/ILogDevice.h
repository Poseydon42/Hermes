#pragma once

#include "Core/Core.h"
#include "Logging/Logger.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	/**
	 * Interface for a thing that can push log output into something
	 */
	class HERMES_API ILogDevice
	{
		MAKE_NON_COPYABLE(ILogDevice)
		
	public:
		virtual ~ILogDevice() = default;

		ILogDevice() = default;

		ILogDevice(ILogDevice&&) = default;

		ILogDevice& operator=(ILogDevice&&) = default;
		
		virtual void Write(LogLevel Level, const String& Text) = 0;

		virtual void WriteLine(LogLevel Level, const String& Text) = 0;

		virtual LogLevel GetCurrentLogLevel() = 0;

		virtual void SetCurrentLogLevel(LogLevel NewLevel) = 0;
	};
}