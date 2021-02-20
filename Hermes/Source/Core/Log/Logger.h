#pragma once

#include <vector>
#include <varargs.h>

#include "Core/Core.h"

namespace Hermes
{
	class ILogDevice;

	enum LogLevel
	{
		Trace,
		Debug,
		Warning,
		Error,
		Fatal
	};

	/**
	 * Main interface to all logging system
	 * Stores a list of all attached log devices and after calling any of log functions formats string
	 * and redirects it to all attached devices
	 */
	class HERMES_API Logger
	{
	public:
		/**
		 * Writes a log message to all available log devices with given log level
		 * @param Text Text to print, could include format that is recognized by CRT sprintf
		 */
		static void Log(LogLevel CurrentLevel, const String& Text, ...);

		static void Trace(const String& Text, ...);

		static void Debug(const String& Text, ...);

		static void Warning(const String& Text, ...);

		static void Error(const String& Text, ...);

		static void Fatal(const String& Text, ...);

		static LogLevel GetLogLevel();

		/**
		 * Set new global log level
		 */
		static void SetLogLevel(LogLevel NewLevel);

		/**
		 * Attach new log device
		 */
		static void AttachLogDevice(ILogDevice* Device);

		/**
		 * Detaches given log device
		 */
		static void DetachLogDevice(ILogDevice* Device);

		/**
		 * Max size of log string after all formating
		 * Everything that exceeds it will be trimmed
		 */
		static const size_t BufferSize = 4095; // + 1 symbol for null terminator
	private:
		static void LogImpl(LogLevel CurrentLevel, const String& Text, va_list Args);

		static std::vector<ILogDevice*> LogDevices;

		static LogLevel CurrentLevel;
	};
}