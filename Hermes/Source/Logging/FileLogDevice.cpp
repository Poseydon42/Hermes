#include "FileLogDevice.h"

namespace Hermes
{
	FileLogDevice::FileLogDevice(const String& Path, LogLevel InitialLevel) : CurrentLevel(InitialLevel)
	{
		Target = PlatformFilesystem::OpenFile(Path, IPlatformFile::FileAccessMode::Write, IPlatformFile::FileOpenMode::Create);
	}

	void FileLogDevice::Write(LogLevel Level, const String& Text)
	{
		if (Level >= CurrentLevel && Target->IsValid())
			Target->Write(reinterpret_cast<const uint8*>(Text.c_str()), Text.length() * sizeof(Text[0]));
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
