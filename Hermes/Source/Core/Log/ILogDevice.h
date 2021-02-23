#pragma once

#include "Core/Core.h"
#include "Core/Log/Logger.h"

namespace Hermes
{
	/**
	 * Interface for a thing that can push log output into something
	 */
	class HERMES_API ILogDevice
	{
	public:
		virtual void Write(LogLevel Level, String Text) = 0;

		virtual void WriteLine(LogLevel Level, String Text) = 0;

		virtual LogLevel GetCurrentLogLevel() = 0;

		virtual void SetCurrentLogLevel(LogLevel NewLevel) = 0;
	};
}