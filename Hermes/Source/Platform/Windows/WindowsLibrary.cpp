#include "WindowsLibrary.h"

#ifdef HERMES_PLATFORM_WINDOWS

#include "Core/Misc/StringUtils.h"

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

	WindowsLibrary& Hermes::WindowsLibrary::operator=(WindowsLibrary&& Other)
	{
		std::swap(Library, Other.Library);
		return *this;
	}

	void* WindowsLibrary::GetSymbolAddress(const String& Name)
	{
		std::string ANSIName = StringUtils::StringToANSI(Name);
		return (void*)GetProcAddress(Library, ANSIName.c_str());
	}

	bool WindowsLibrary::IsValid()
	{
		return (Library != INVALID_HANDLE_VALUE);
	}

	WindowsLibrary::WindowsLibrary(const String& Path)
	{
		Library = LoadLibraryW(Path.c_str());
	}

	std::shared_ptr<IPlatformLibrary> IPlatformLibrary::Load(const String& Path)
	{
		return std::make_shared<WindowsLibrary>(Path);
	}
}

#endif
