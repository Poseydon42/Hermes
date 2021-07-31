#pragma once

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"
#include "Vulkan.h"

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
			~VulkanPhysicalDevice() override;
			VulkanPhysicalDevice(VulkanPhysicalDevice&&);
			VulkanPhysicalDevice& operator=(VulkanPhysicalDevice&&);

			const RenderInterface::DeviceProperties& GetProperties() const override;
			std::shared_ptr<RenderInterface::Device> CreateDevice() override;

			VkInstance GetInstance();
			
		private:
			VulkanPhysicalDevice(VkPhysicalDevice InDevice, VkInstance InInstance, VkSurfaceKHR InSurface);
			
			VkPhysicalDevice Device;
			VkInstance Instance;
			VkSurfaceKHR Surface;
			RenderInterface::DeviceProperties Properties;

			friend class VulkanInstance;
		};
	}
}
