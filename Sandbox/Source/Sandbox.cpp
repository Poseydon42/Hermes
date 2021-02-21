#ifdef HERMES_PLATFORM_WINDOWS

#include <windows.h>

#include "Core/Core.h"
#include "Platform/GenericPlatform/PlatformDebug.h"
#include "Core/Application/Application.h"
#include "Core/Log/Logger.h"

class SandboxApp : public Hermes::IApplication
{
public:
	bool Init() override
	{
		HERMES_LOG_DEBUG(L"Some text, here's an 32 bit hexadecimal integer %#010X and a float %f", 0x1234FFDD, 42.0f);
		return true;
	}

	void Run(float Delta) override
	{
		
	}

	void Shutdown() override
	{
		OutputDebugString(TEXT("SandboxApp shutdown\n"));
	}
};

extern "C" _declspec(dllexport) Hermes::IApplication* CreateApplicationInstance()
{
	auto App = new SandboxApp;

	return App;
}

#endif
