#include "Core/Application/EventQueue.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#ifdef HERMES_PLATFORM_WINDOWS

#include "Core/Core.h"
#include "Core/Application/Application.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
#include "Platform/GenericPlatform/PlatformLibrary.h"
#include "RenderInterface/GenericRenderInterface/Resource.h"
#include "RenderInterface/GenericRenderInterface/Instance.h"
#include "RenderInterface/Vulkan/VulkanInstance.h"
#include "RenderInterface/Vulkan/VulkanSwapchain.h"
#include "Core/Application/GameLoop.h"

void WindowEventHandler(const Hermes::IEvent& Event)
{
	HERMES_LOG_INFO(L"Received window event: %s", Event.ToString());
}

class SandboxApp : public Hermes::IApplication
{
public:
	bool Init() override
	{
		Hermes::RenderInterface::Instance* RenderInstance = new Hermes::Vulkan::VulkanInstance(*Hermes::GGameLoop->GetWindow());
		auto Devices = RenderInstance->EnumerateAvailableDevices();
		std::shared_ptr<Hermes::RenderInterface::PhysicalDevice> PhysicalDevice = RenderInstance->GetPhysicalDevice(Devices[0].InternalIndex);

		auto GPUs = RenderInstance->EnumerateAvailableDevices();
		auto Device = PhysicalDevice->CreateDevice();

		auto Swapchain = Device->CreateSwapchain({ 1280, 720 }, 3);

		auto Buffer = Device->CreateBuffer(1024, Hermes::RenderInterface::ResourceUsageType::VertexBuffer | Hermes::RenderInterface::ResourceUsageType::CPUAccessible);
		void* BufferMemory = Buffer->Map();
		HERMES_ASSERT(BufferMemory);
		HERMES_ASSERT(Buffer->GetSize() == 1024);
		HERMES_ASSERT(Buffer->GetResourceType() == Hermes::RenderInterface::ResourceType::Buffer);
		Buffer->Unmap();

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
