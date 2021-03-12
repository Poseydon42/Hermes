#include "Core/Application/EventQueue.h"
#ifdef HERMES_PLATFORM_WINDOWS

#include "Core/Core.h"
#include "Core/Application/Application.h"
#include "Platform/GenericPlatform/PlatformWindow.h"

void WindowEventHandler(const Hermes::IEvent& Event)
{
	HERMES_LOG_INFO(L"Received window event: %s", Event.ToString());
}

class SandboxApp : public Hermes::IApplication
{
public:
	bool Init() override
	{
		ApplicationWindow = Hermes::IPlatformWindow::CreatePlatformWindow(L"Hermes Engine", Hermes::Vec2i(1280, 720));
		WindowMessageQueue = ApplicationWindow->WindowQueue();
		if (auto Queue = WindowMessageQueue.lock())
		{
			Queue->Subscribe<WindowEventHandler>(Hermes::WindowCloseEvent::GetStaticType());
		}
		return true;
	}

	void Run(float Delta) override
	{
	}

	void Shutdown() override
	{
		
	}

private:
	std::shared_ptr<Hermes::IPlatformWindow> ApplicationWindow;

	std::weak_ptr<Hermes::EventQueue> WindowMessageQueue;
};

extern "C" _declspec(dllexport) Hermes::IApplication* CreateApplicationInstance()
{
	auto App = new SandboxApp;

	return App;
}

#endif
