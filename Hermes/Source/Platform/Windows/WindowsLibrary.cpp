#include "WindowsLibrary.h"

#ifdef HERMES_PLATFORM_WINDOWS

namespace Hermes
{
	WindowsLibrary::~WindowsLibrary()
	{
		FreeLibrary(Library);
	}

	WindowsLibrary::WindowsLibrary(WindowsLibrary&& Other)
	{
		std::swap(Library, Other.Library);
	}

	WindowsLibrary& WindowsLibrary::operator=(WindowsLibrary&& Other)
	{
		std::swap(Library, Other.Library);
		return *this;
	}

	void* WindowsLibrary::GetSymbolAddress(const String& Name)
	{
		return static_cast<void*>(GetProcAddress(Library, Name.c_str()));
	}

	bool WindowsLibrary::IsValid()
	{
		return (Library != INVALID_HANDLE_VALUE);
	}

	WindowsLibrary::WindowsLibrary(const String& Path)
	{
		static constexpr size_t MaxLibraryPathLength = 8192;
		wchar_t Buffer[MaxLibraryPathLength];
		MultiByteToWideChar(CP_UTF8, 0, Path.c_str(), -1, Buffer, MaxLibraryPathLength);
		Library = LoadLibraryW(Buffer);
	}

	std::unique_ptr<IPlatformLibrary> IPlatformLibrary::Load(const String& Path)
	{
		return std::make_unique<WindowsLibrary>(Path);
	}
}

#endif
