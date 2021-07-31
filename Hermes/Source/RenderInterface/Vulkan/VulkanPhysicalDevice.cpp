#include "VulkanPhysicalDevice.h"

#include "RenderInterface/Vulkan/VulkanDevice.h"

namespace Hermes
{
	namespace Vulkan
	{
		RenderInterface::DeviceProperties GetPhysicalDeviceProperties(VkPhysicalDevice Device)
		{
			RenderInterface::DeviceProperties Result;
			
			VkPhysicalDeviceProperties Properties;

			vkGetPhysicalDeviceProperties(Device, &Properties);
			Result.Name = Properties.deviceName;

			return Result;
		}

		VulkanPhysicalDevice::~VulkanPhysicalDevice()
		{
		}

		VulkanPhysicalDevice::VulkanPhysicalDevice(VulkanPhysicalDevice&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(Instance, Other.Instance);
			std::swap(Surface, Other.Surface);
			std::swap(Properties, Other.Properties);
		}

		VulkanPhysicalDevice& VulkanPhysicalDevice::operator=(VulkanPhysicalDevice&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(Instance, Other.Instance);
			std::swap(Surface, Other.Surface);
			std::swap(Properties, Other.Properties);
			return *this;
		}

		const RenderInterface::DeviceProperties& VulkanPhysicalDevice::GetProperties() const
		{
			return Properties;
		}

		std::shared_ptr<RenderInterface::Device> VulkanPhysicalDevice::CreateDevice()
		{
			return std::make_shared<VulkanDevice>(Device, Instance, Surface);
		}

		VkInstance VulkanPhysicalDevice::GetInstance()
		{
			return Instance;
		}

		VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice InDevice, VkInstance InInstance, VkSurfaceKHR InSurface) : Device(InDevice), Instance(InInstance), Properties(GetPhysicalDeviceProperties(InDevice)), Surface(InSurface)
		{
		}
	}
}
