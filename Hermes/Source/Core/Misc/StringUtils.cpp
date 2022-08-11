#include "StringUtils.h"

#include <cstring>

#ifdef HERMES_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <cstdlib>
#endif

namespace Hermes
{
	String StringUtils::ANSIToString(const ANSIString& In)
	{
		return ANSIToString(In.c_str());
	}

	String StringUtils::ANSIToString(const char* In)
	{
		size_t Length = strlen(In);
		String Result(Length, 0);
#ifdef HERMES_PLATFORM_WINDOWS
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, In, -1, Result.data(), static_cast<int>(Length));
#else
		// TODO : implement properly
		// std::wstring seems not to be completely cross-platform, we should deal with it when we actually move to Linux systems
#endif
		return Result;
	}

	ANSIString StringUtils::StringToANSI(const String& In)
	{
		ANSIString Result(In.size(), 0);
#ifdef HERMES_PLATFORM_WINDOWS
		WideCharToMultiByte(CP_ACP, 0, In.c_str(), -1, Result.data(), (int)Result.size(), NULL, NULL);
#else
		// TODO : unimplemented
#endif
		return Result;
	}
}
