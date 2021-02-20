#ifdef HERMES_PLATFORM_WINDOWS

#include <windows.h>

#include "Platform/GenericPlatform/PlatformDebug.h"

namespace Hermes
{
	void PlatformDebug::PrintString(String Text)
	{
		OutputDebugStringW(Text.c_str());
	}
}

#endif
