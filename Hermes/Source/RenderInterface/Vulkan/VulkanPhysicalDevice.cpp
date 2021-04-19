#include "VulkanPhysicalDevice.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanPhysicalDevice::~VulkanPhysicalDevice()
		{
		}

		VulkanPhysicalDevice::VulkanPhysicalDevice(VulkanPhysicalDevice&& Other)
		{
			std::swap(Device, Other.Device);
		}

		VulkanPhysicalDevice& VulkanPhysicalDevice::operator=(VulkanPhysicalDevice&& Other)
		{
			std::swap(Device, Other.Device);
			return *this;
		}

		VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice InDevice) : Device(InDevice)
		{
		}
	}
}
