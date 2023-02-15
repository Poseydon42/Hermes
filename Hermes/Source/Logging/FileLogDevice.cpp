#include "FileLogDevice.h"

#include "VirtualFilesystem/VirtualFilesystem.h"

namespace Hermes
{
	FileLogDevice::FileLogDevice(const String& Path, LogLevel InitialLevel) : CurrentLevel(InitialLevel)
	{
		Target = VirtualFilesystem::Open(Path, FileOpenMode::CreateNew, FileAccessMode::ReadWrite);
	}

	void FileLogDevice::Write(LogLevel Level, const String& Text)
	{
		if (Level >= CurrentLevel)
			Target->Write(Text.c_str(), Text.length() * sizeof(Text[0]));
	}

	void FileLogDevice::WriteLine(LogLevel Level, const String& Text)
	{
		Write(Level, Text);
		Write(Level, "\n");
	}

	LogLevel FileLogDevice::GetCurrentLogLevel()
	{
		return CurrentLevel;
	}

	void FileLogDevice::SetCurrentLogLevel(LogLevel NewLevel)
	{
		CurrentLevel = NewLevel;
	}
}
