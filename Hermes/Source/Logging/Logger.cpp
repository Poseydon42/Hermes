#include "Logger.h"

#include <cstdarg>

#include "Logging/ILogDevice.h"
#include "Platform/GenericPlatform/PlatformTime.h"

namespace Hermes
{
	std::vector<ILogDevice*> Logger::LogDevices;

	LogLevel Logger::CurrentLevel;

	String Logger::CurrentFormat;

	void Logger::LogImpl(LogLevel Level, const char* Filename, int32 Line, const char* Text, va_list Args)
	{
		char MessageBuffer[BufferSize + 1];
		char FinalBuffer[BufferSize + 1];

		if (Level < CurrentLevel)
			return;

		std::vsnprintf(MessageBuffer, BufferSize + 1, Text, Args);
		ApplyFormatting(Level, Filename, Line, FinalBuffer, BufferSize + 1, MessageBuffer);
		for (auto Device : LogDevices)
		{
			Device->WriteLine(Level, FinalBuffer);
		}
	}

	void Logger::ApplyFormatting(LogLevel Level, const char* Filename, int32 Line, char* Buffer, size_t BufferCount, const char* Message)
	{
		memset(Buffer, 0, BufferCount * sizeof(Buffer[0]));
		const char* String = CurrentFormat.c_str();
		char* Dest = Buffer;
		// TODO: does this work well with UTF-8?
		while (*String)
		{
			size_t SpaceLeft = BufferCount - (Dest - Buffer) - 1;
			size_t SpaceTaken = 0;
			if (SpaceLeft >= BufferCount)
				return;
			if (*String == '%')
			{
				String++;
				switch (*String)
				{
				case '%':
					SpaceTaken = 1;
					*Dest = '%';
					break;
				case 'v': // Actual message
				{
					SpaceTaken = std::snprintf(Dest, SpaceLeft + 1, "%s", Message);
					break;
				}
				case 'l':
				{
					const char* Lookup[] = {
						"Trace",
						"Debug",
						"Info",
						"Warning",
						"Error",
						"Fatal"
					};
					SpaceTaken = std::snprintf(Dest, SpaceLeft + 1, "%s", Lookup[static_cast<size_t>(Level)]);
					break;
				}
				case 'h':
				case 'm':
				case 's':
				case 'u':
				case 'y':
				case 'Y':
				case 'M':
				case 'd':
				{
					// Avoiding many calls to GetPlatformTime() as it may be very time consuming
					PlatformDateTime Time = PlatformTime::GetPlatformTime();
					switch (*String)
					{
					case 'h':
						SpaceTaken = std::snprintf(Dest, SpaceLeft + 1, "%02hu", Time.Hour);
						break;
					case 'm':
						SpaceTaken = std::snprintf(Dest, SpaceLeft + 1, "%02hu", Time.Minute);
						break;
					case 's':
						SpaceTaken = std::snprintf(Dest, SpaceLeft + 1, "%02hu", Time.Second);
						break;
					case 'u':
						SpaceTaken = std::snprintf(Dest, SpaceLeft + 1, "%03hu", Time.Millisecond);
						break;
					case 'y':
						SpaceTaken = std::snprintf(Dest, SpaceLeft + 1, "%02hu", Time.Year % 1000);
						break;
					case 'Y':
						SpaceTaken = std::snprintf(Dest, SpaceLeft + 1, "%04hu", Time.Year);
						break;
					case 'M':
						SpaceTaken = std::snprintf(Dest, SpaceLeft + 1, "%02hu", Time.Month);
						break;
					case 'd':
						SpaceTaken = std::snprintf(Dest, SpaceLeft + 1, "%02hu", Time.Day);
						break;
					}
					break;
				}
				case 'f':
					SpaceTaken = std::snprintf(Dest, SpaceLeft + 1, "%s", Filename);
					break;
				case '#':
					SpaceTaken = std::snprintf(Dest, SpaceLeft + 1, "%d", Line);
					break;
				}
			}
			else
			{
				*Dest = *String;
				SpaceTaken = 1;
			}
			Dest += SpaceTaken;
			String++;
		}
	}

	void Logger::Log(LogLevel Level, const char* Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(Level, "", 0, Text, Args);
		va_end(Args);
	}

	void Logger::LogWithFilename(LogLevel Level, const char* Filename, int32 Line, const char* Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(Level, Filename, Line, Text, Args);
		va_end(Args);
	}

	void Logger::Trace(const char* Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(LogLevel::Trace, "", 0, Text, Args);
		va_end(Args);
	}

	void Logger::Debug(const char* Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(LogLevel::Debug, "", 0, Text, Args);
		va_end(Args);
	}

	void Logger::Info(const char* Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(LogLevel::Info, "", 0, Text, Args);
		va_end(Args);
	}

	void Logger::Warning(const char* Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(LogLevel::Warning, "", 0, Text, Args);
		va_end(Args);
	}

	void Logger::Error(const char* Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(LogLevel::Error, "", 0, Text, Args);
		va_end(Args);
	}

	void Logger::Fatal(const char* Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(LogLevel::Fatal, "", 0, Text, Args);
		va_end(Args);
	}

	LogLevel Logger::GetLogLevel()
	{
		return CurrentLevel;
	}

	void Logger::SetLogLevel(LogLevel NewLevel)
	{
		CurrentLevel = NewLevel;
	}

	void Logger::AttachLogDevice(ILogDevice* Device)
	{
		LogDevices.push_back(Device);
	}

	void Logger::DetachLogDevice(ILogDevice* Device)
	{
		auto FoundDevice = std::find(LogDevices.begin(), LogDevices.end(), Device);
		if (FoundDevice != LogDevices.end())
		{
			LogDevices.erase(FoundDevice);
		}
	}

	void Logger::SetLogFormat(const String& NewFormat)
	{
		CurrentFormat = NewFormat;
	}
}
