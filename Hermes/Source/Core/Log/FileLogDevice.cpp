#include "FileLogDevice.h"

namespace Hermes
{
	FileLogDevice::FileLogDevice(const String& Path, LogLevel InitialLevel) : CurrentLevel(InitialLevel)
	{
		Target = PlatformFilesystem::OpenFile(Path, IPlatformFile::FileAccessMode::Write, IPlatformFile::FileOpenMode::Create);
	}

	void FileLogDevice::Write(LogLevel Level, String Text)
	{
		if (Level >= CurrentLevel)
			Target->Write((const uint8*)Text.c_str(), Text.length() * sizeof(Text[0]));
	}

	void FileLogDevice::WriteLine(LogLevel Level, String Text)
	{
		if (Level >= CurrentLevel)
		{
			Text[Text.length() - 1] = L'\n'; // Overwrite null-terminator with line ending
			Write(Level, Text);
		}
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
