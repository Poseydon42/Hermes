#ifdef HERMES_PLATFORM_WINDOWS

#include "Core/Core.h"
#include "Core/Application/Application.h"
#include "Platform/GenericPlatform/PlatformWindow.h"

class SandboxApp : public Hermes::IApplication
{
public:
	bool Init() override
	{
		std::shared_ptr<Hermes::IPlatformWindow> Window = Hermes::IPlatformWindow::CreatePlatformWindow(L"Hermes Engine", Hermes::Vec2i(1280, 720));
		
		return true;
	}

	void Run(float Delta) override
	{
	}

	void Shutdown() override
	{
		
	}
};

extern "C" _declspec(dllexport) Hermes::IApplication* CreateApplicationInstance()
{
	auto App = new SandboxApp;

	return App;
}

#endif
