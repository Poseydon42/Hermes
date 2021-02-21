#include "Logger.h"

#include <stdarg.h>

#include "Core/Log/ILogDevice.h"

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
		ApplyFormating(FinalBuffer, BufferSize + 1, MessageBuffer);
		for (auto Device : LogDevices)
		{
			Device->WriteLine(Level, FinalBuffer);
		}
	}

	void Logger::ApplyFormating(wchar_t* Buffer, size_t BufferCount, const wchar_t* Message)
	{
		memcpy(Buffer, Message, BufferCount * sizeof(Buffer[0]));
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
}
