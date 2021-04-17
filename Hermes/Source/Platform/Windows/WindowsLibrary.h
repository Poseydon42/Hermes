#pragma once

#ifdef HERMES_PLATFORM_WINDOWS

#include <Windows.h>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Platform/GenericPlatform/PlatformLibrary.h"

namespace Hermes
{
	class HERMES_API WindowsLibrary : public IPlatformLibrary
	{
		MAKE_NON_COPYABLE(WindowsLibrary)
		
	public:
		WindowsLibrary(const String& Path);
		
		~WindowsLibrary() override;

		WindowsLibrary(WindowsLibrary&& Other);

		WindowsLibrary& operator=(WindowsLibrary&& Other);
		
		void* GetSymbolAddress(const String& Name) override;

		bool IsValid() override;
	private:

		HMODULE Library;
	};
}

#endif
