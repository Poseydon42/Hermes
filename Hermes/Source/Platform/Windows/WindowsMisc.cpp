#ifdef HERMES_PLATFORM_WINDOWS

#include "Platform/GenericPlatform/PlatformMisc.h"

#include <windows.h>

namespace Hermes
{
	void PlatformMisc::Exit(uint32 ExitCode)
	{
		ExitProcess(ExitCode);
	}

	void PlatformMisc::ExitWithMessageBox(uint32 ExitCode, const String& Title, const String& Message)
	{
		MessageBoxW(0, Message.c_str(), Title.c_str(), MB_OK | MB_ICONERROR);
		Exit(ExitCode);
	}
}

#endif