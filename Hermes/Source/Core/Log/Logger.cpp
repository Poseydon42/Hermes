#include "Logger.h"

namespace Hermes
{
	std::vector<ILogDevice*> Logger::LogDevices;

	LogLevel Logger::CurrentLevel;

	void Logger::LogImpl(LogLevel Level, const String& Text, va_list Args)
	{
		wchar_t Buffer[BufferSize];

		vswprintf(Buffer, Text.c_str(), Args);

		if (Level < CurrentLevel)
			return;

		for (auto Device : LogDevices)
		{
			Device->WriteLine(Buffer);
		}
	}

	void Logger::Log(LogLevel Level, const String& Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(Level, Text, Args);
		va_end(Args);
	}

	void Logger::Trace(const String& Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(LogLevel::Trace, Text, Args);
		va_end(Args);
	}

	void Logger::Debug(const String& Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(LogLevel::Debug, Text, Args);
		va_end(Args);
	}

	void Logger::Warning(const String& Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(LogLevel::Warning, Text, Args);
		va_end(Args);
	}

	void Logger::Error(const String& Text, ...)
	{
		va_list Args;
		va_start(Args, Text);
		LogImpl(LogLevel::Error, Text, Args);
		va_end(Args);
	}

	void Logger::Fatal(const String& Text, ...)
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
}