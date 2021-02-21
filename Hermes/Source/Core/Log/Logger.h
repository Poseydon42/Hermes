#pragma once

#include <vector>

#include "Core/Core.h"

namespace Hermes
{
	class ILogDevice;

	enum class LogLevel
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
		static void Log(LogLevel CurrentLevel, const wchar_t* Text, ...);

		static void Trace(const wchar_t* Text, ...);

		static void Debug(const wchar_t* Text, ...);

		static void Warning(const wchar_t* Text, ...);

		static void Error(const wchar_t* Text, ...);

		static void Fatal(const wchar_t* Text, ...);

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
		static void LogImpl(LogLevel CurrentLevel, const wchar_t* Text, va_list Args);

		static void ApplyFormating(wchar_t* Buffer, size_t BufferCount, const wchar_t* Message);

		static std::vector<ILogDevice*> LogDevices;

		static LogLevel CurrentLevel;

		static String CurrentFormat;
	};
}

#define HERMES_LOG_TRACE(Text, ...)   ::Hermes::Logger::Trace((Text), __VA_ARGS__)
#define HERMES_LOG_DEBUG(Text, ...)   ::Hermes::Logger::Debug((Text), __VA_ARGS__)
#define HERMES_LOG_WARNING(Text, ...) ::Hermes::Logger::Warning((Text), __VA_ARGS__)
#define HERMES_LOG_ERROR(Text, ...)   ::Hermes::Logger::Error((Text), __VA_ARGS__)
#define HERMES_LOG_FATAL(Text, ...)   ::Hermes::Logger::Fatal((Text), __VA_ARGS__)
