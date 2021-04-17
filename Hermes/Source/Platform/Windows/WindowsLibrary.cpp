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

	WindowsLibrary& Hermes::WindowsLibrary::operator=(WindowsLibrary&& Other)
	{
		std::swap(Library, Other.Library);
		return *this;
	}

	void* WindowsLibrary::GetSymbolAddress(const String& Name)
	{
		std::string ANSIName(Name.begin(), Name.end());
		return (void*)GetProcAddress(Library, ANSIName.c_str());
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
