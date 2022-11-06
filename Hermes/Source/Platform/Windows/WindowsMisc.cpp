#ifdef HERMES_PLATFORM_WINDOWS

#include "Platform/GenericPlatform/PlatformMisc.h"

#include <Windows.h>

namespace Hermes
{
	void PlatformMisc::Exit(uint32 ExitCode)
	{
		ExitProcess(ExitCode);
	}

	void PlatformMisc::ExitWithMessageBox(uint32 ExitCode, const String& Title, const String& Message)
	{
		static constexpr size_t BufferSize = 2048;
		wchar_t MessageBuffer[BufferSize], TitleBuffer[BufferSize];

		MultiByteToWideChar(CP_UTF8, 0, Title.c_str(), -1, TitleBuffer, BufferSize);
		MultiByteToWideChar(CP_UTF8, 0, Message.c_str(), -1, MessageBuffer, BufferSize);
		MessageBoxW(0, MessageBuffer, TitleBuffer, MB_OK | MB_ICONERROR);
		Exit(ExitCode);
	}
}

#endif