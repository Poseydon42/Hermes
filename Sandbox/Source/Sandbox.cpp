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
		auto Devices = RenderInstance->EnumerateAvailableDevices();
		std::shared_ptr<Hermes::RenderInterface::PhysicalDevice> PhysicalDevice = RenderInstance->GetPhysicalDevice(Devices[0].InternalIndex);

		auto GPUs = RenderInstance->EnumerateAvailableDevices();
		std::vector<Hermes::RenderInterface::QueueFamilyProperties> Families = PhysicalDevice->GetProperties().QueueFamilies;
		Hermes::RenderInterface::QueueFamilyProperties Queue = {};
		Queue.Index = 0;
		Queue.Count = 1;
		Queue.Type = Families[0].Type;
		std::vector<Hermes::RenderInterface::QueueFamilyProperties> RequiredQueues(1, Queue);
		auto Device = PhysicalDevice->CreateDevice(RequiredQueues);

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
