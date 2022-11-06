#ifdef HERMES_PLATFORM_WINDOWS

#include <Windows.h>

#include "Platform/GenericPlatform/PlatformDebug.h"

namespace Hermes
{
	void PlatformDebug::PrintString(const String& Text)
	{
		// This is probably a huge overestimate, but this code should not be used in release or tight loops anyway
		size_t BufferSize = Text.size() * 2 + 2;
		auto* Buffer = new wchar_t[BufferSize];
		MultiByteToWideChar(CP_UTF8, 0, Text.c_str(), -1, Buffer, static_cast<int>(BufferSize));
		OutputDebugStringW(Buffer);
		delete[] Buffer;
	}
}

#endif
