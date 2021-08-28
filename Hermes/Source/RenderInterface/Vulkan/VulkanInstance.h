#pragma once

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Instance.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class HERMES_API VulkanInstance final : public RenderInterface::Instance, public std::enable_shared_from_this<VulkanInstance>
		{
			MAKE_NON_COPYABLE(VulkanInstance)
		public:
			VulkanInstance(const IPlatformWindow& Window);
			
			~VulkanInstance() override;

			VulkanInstance(VulkanInstance&& Other);
			VulkanInstance& operator=(VulkanInstance&& Other);

			std::vector<RenderInterface::DeviceProperties> EnumerateAvailableDevices() override;
			
			std::shared_ptr<RenderInterface::PhysicalDevice> GetPhysicalDevice(RenderInterface::DeviceIndex Index) override;

			VkInstance GetInstance() const { return Instance; }
			
		private:
			void CreateDebugMessenger();
			
			VkInstance Instance = VK_NULL_HANDLE;
			VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;
			VkSurfaceKHR Surface = VK_NULL_HANDLE;

			std::vector<VkPhysicalDevice> AvailableDevices;
			std::vector<RenderInterface::DeviceProperties> AvailableDeviceProperties;
		};
	}
}
