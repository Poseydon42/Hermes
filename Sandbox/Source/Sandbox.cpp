#include "Core/Application/EventQueue.h"
#ifdef HERMES_PLATFORM_WINDOWS

#include "Core/Core.h"
#include "Core/Application/Application.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
#include "Platform/GenericPlatform/PlatformLibrary.h"
#include "RenderInterface/GenericRenderInterface/Instance.h"
#include "RenderInterface/Vulkan/VulkanInstance.h"
#include "Core/Misc/StringUtils.h"

void WindowEventHandler(const Hermes::IEvent& Event)
{
	HERMES_LOG_INFO(L"Received window event: %s", Event.ToString());
}

class SandboxApp : public Hermes::IApplication
{
public:
	bool Init() override
	{
		Hermes::RenderInterface::Instance* RenderInstance = new Hermes::Vulkan::VulkanInstance();

		Hermes::String Str1 = L"123рст";
		Hermes::ANSIString Str2 = Hermes::StringUtils::StringToANSI(Str1);
		Hermes::String Str3 = Hermes::StringUtils::ANSIToString(Str2);
		Hermes::ANSIString Str4 = Hermes::StringUtils::StringToANSI(Str3);
		Hermes::String FinalStr = Hermes::StringUtils::ANSIToString(Str4);

		HERMES_ASSERT(Str1 == FinalStr);

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
