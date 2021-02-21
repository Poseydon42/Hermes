#include "Logger.h"

#include <stdarg.h>
#include <cwchar>

#include "Core/Log/ILogDevice.h"
#include "Platform/GenericPlatform/PlatformTime.h"

namespace Hermes
{
	std::vector<ILogDevice*> Logger::LogDevices;

	LogLevel Logger::CurrentLevel;

	String Logger::CurrentFormat;

	void Logger::LogImpl(LogLevel Level, const wchar_t* Text, va_list Args)
	{
		wchar_t MessageBuffer[BufferSize + 1];
		wchar_t FinalBuffer[BufferSize + 1];

		if (Level < CurrentLevel)
			return;

		vswprintf(MessageBuffer, BufferSize + 1, Text, Args);
		ApplyFormating(Level, FinalBuffer, BufferSize + 1, MessageBuffer);
		for (auto Device : LogDevices)
		{
			Device->WriteLine(Level, FinalBuffer);
		}
	}

	void Logger::ApplyFormating(LogLevel Level, wchar_t* Buffer, size_t BufferCount, const wchar_t* Message)
	{
		memset(Buffer, 0, BufferCount * sizeof(Buffer[0]));
		const wchar_t* s = CurrentFormat.c_str();
		wchar_t* t = Buffer;
		while (*s)
		{
			size_t SpaceLeft = BufferCount - (t - Buffer) - 1;
			size_t SpaceTaken = 0;
			if (SpaceLeft >= BufferCount)
				return;
			if (*s == L'%')
			{
				s++;
				switch (*s)
				{
				case L'v': // Actual message
				{
					SpaceTaken = swprintf_s(t, SpaceLeft + 1, L"%s", Message);
					break;
				}
				case L'l':
				{
					const wchar_t* Lookup[] = {
						L"Trace",
						L"Debug",
						L"Warning",
						L"Error",
						L"Fatal"
					};
					SpaceTaken = swprintf_s(t, SpaceLeft + 1, L"%s", Lookup[(size_t)Level]);
					break;
				}
				case L'h':
				case L'm':
				case L's':
				case L'u':
				case L'y':
				case L'Y':
				case L'M':
				case L'd':
				{
					// Avoiding many calls to GetPlatformTime() as it may be very time consuming
					PlatformTimestamp Time = PlatformTime::GetPlatformTime();
					switch (*s)
					{
					case L'h':
						SpaceTaken = swprintf_s(t, SpaceLeft + 1, L"%02hu", Time.Hour);
						break;
					case L'm':
						SpaceTaken = swprintf_s(t, SpaceLeft + 1, L"%02hu", Time.Minute);
						break;
					case L's':
						SpaceTaken = swprintf_s(t, SpaceLeft + 1, L"%02hu", Time.Second);
						break;
					case L'u':
						SpaceTaken = swprintf_s(t, SpaceLeft + 1, L"%03hu", Time.Milisecond);
						break;
					case L'y':
						SpaceTaken = swprintf_s(t, SpaceLeft + 1, L"%02hu", Time.Year % 1000);
						break;
					case L'Y':
						SpaceTaken = swprintf_s(t, SpaceLeft + 1, L"%04hu", Time.Year);
						break;
					case L'M':
						SpaceTaken = swprintf_s(t, SpaceLeft + 1, L"%02hu", Time.Month);
						break;
					case L'd':
						SpaceTaken = swprintf_s(t, SpaceLeft + 1, L"%02hu", Time.Day);
						break;
					}
					break;
				}
				}
			}
			else
			{
				*t = *s;
				SpaceTaken = 1;
			}
			t += SpaceTaken;
			s++;
		}
	}

	void Logger::Log(LogLevel Level, const wchar_t* Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(Level, Text, Args);
		va_end(Args);
	}

	void Logger::Trace(const wchar_t* Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(LogLevel::Trace, Text, Args);
		va_end(Args);
	}

	void Logger::Debug(const wchar_t* Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(LogLevel::Debug, Text, Args);
		va_end(Args);
	}

	void Logger::Warning(const wchar_t* Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(LogLevel::Warning, Text, Args);
		va_end(Args);
	}

	void Logger::Error(const wchar_t* Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(LogLevel::Error, Text, Args);
		va_end(Args);
	}

	void Logger::Fatal(const wchar_t* Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(LogLevel::Fatal, Text, Args);
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
