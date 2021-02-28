#ifdef HERMES_PLATFORM_WINDOWS

#include <windows.h>

#include "Core/Core.h"
#include "Platform/GenericPlatform/PlatformDebug.h"
#include "Core/Application/Application.h"
#include "Core/Log/Logger.h"
#include "Core/Application/Event.h"
#include "Core/Application/EventQueue.h"

#include "Core/Delegate/Delegate.h"

class CustomEvent : public Hermes::IEvent
{
	EVENT_BODY(CustomEvent);
public:
	CustomEvent(const Hermes::String& T) : Text(T) {}

	Hermes::String ToString() const override
	{
		return Text;
	}
	
private:
	Hermes::String Text;
};

void CustomEventListener(const Hermes::IEvent& Event)
{
	HERMES_LOG_DEBUG(L"Received CustomEvent: %s", Event.ToString().c_str());
}

class SandboxApp : public Hermes::IApplication
{
public:
	bool Init() override
	{
		HERMES_LOG_DEBUG(L"Some text, here's an 32 bit hexadecimal integer %#010X and a float %f", 0x1234FFDD, 42.0f);
		GameEventQueue.Subscribe<CustomEventListener>(CustomEvent::GetStaticType());
		CustomEvent* Event = new CustomEvent(L"SandboxApp::Init");
		GameEventQueue.PushEvent(*Event);
		delete Event;
		
		return true;
	}

	void Run(float Delta) override
	{
		GameEventQueue.Run();
	}

	void Shutdown() override
	{
		OutputDebugString(TEXT("SandboxApp shutdown\n"));
	}
private:
	Hermes::EventQueue GameEventQueue;
};

extern "C" _declspec(dllexport) Hermes::IApplication* CreateApplicationInstance()
{
	auto App = new SandboxApp;

	return App;
}

#endif
