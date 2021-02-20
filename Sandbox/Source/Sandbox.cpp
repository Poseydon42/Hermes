#ifdef HERMES_PLATFORM_WINDOWS

#include <windows.h>

#include "Core/Core.h"
#include "Core/Application/Application.h"

class SandboxApp : public Hermes::IApplication
{
public:
	bool Init() override
	{
		OutputDebugString(TEXT("SandboxApp init\n"));

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
