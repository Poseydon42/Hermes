#pragma once

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"
#include "Vulkan.h"

namespace Hermes
{
	class IPlatformWindow;
}

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanInstance;

		/**
		 * Fills DeviceProperties struct with actual properties of given device
		 * NOTE : InternalIndex field remains uninitialized
		 */
		RenderInterface::DeviceProperties GetPhysicalDeviceProperties(VkPhysicalDevice Device);
		
		class HERMES_API VulkanPhysicalDevice final : public RenderInterface::PhysicalDevice
		{
			MAKE_NON_COPYABLE(VulkanPhysicalDevice)
		public:
			VulkanPhysicalDevice(VkPhysicalDevice InDevice, std::shared_ptr<const VulkanInstance> InInstance, VkSurfaceKHR InSurface, std::weak_ptr<const IPlatformWindow> InWindow);
			
			~VulkanPhysicalDevice() override = default;
			VulkanPhysicalDevice(VulkanPhysicalDevice&&);
			VulkanPhysicalDevice& operator=(VulkanPhysicalDevice&&);

			const RenderInterface::DeviceProperties& GetProperties() const override;
			std::shared_ptr<RenderInterface::Device> CreateDevice() const override;

			VkInstance GetInstance() const;
			
		private:
			VkPhysicalDevice Device;
			std::shared_ptr<const VulkanInstance> Instance;
			VkSurfaceKHR Surface;
			RenderInterface::DeviceProperties Properties;
			std::weak_ptr<const IPlatformWindow> Window;
		};
	}
}
