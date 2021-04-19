#include "VulkanPhysicalDevice.h"

namespace Hermes
{
	namespace Vulkan
	{
		RenderInterface::DeviceProperties GetPhysicalDeviceProperties(VkPhysicalDevice Device)
		{
			RenderInterface::DeviceProperties Result;
			
			VkPhysicalDeviceProperties Properties;
			std::vector<VkQueueFamilyProperties> QueueFamilies;
			uint32_t QueueFamiliesCount = 0;

			vkGetPhysicalDeviceProperties(Device, &Properties);
			Result.Name = Properties.deviceName;

			vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamiliesCount, 0);
			QueueFamilies.resize(QueueFamiliesCount);
			vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamiliesCount, QueueFamilies.data());

			Result.QueueFamilies.reserve(QueueFamiliesCount);
			for (auto& Family : QueueFamilies)
			{
				RenderInterface::QueueFamilyProperties QueueFamilyProps = {};
				QueueFamilyProps.Count = Family.queueCount;
				if (Family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
					QueueFamilyProps.Type |= RenderInterface::QueueFamilyType::Graphics;
				if (Family.queueFlags & VK_QUEUE_TRANSFER_BIT)
					QueueFamilyProps.Type |= RenderInterface::QueueFamilyType::Transfer;
				Result.QueueFamilies.push_back(QueueFamilyProps);
			}

			return Result;
		}

		VulkanPhysicalDevice::~VulkanPhysicalDevice()
		{
		}

		VulkanPhysicalDevice::VulkanPhysicalDevice(VulkanPhysicalDevice&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(Properties, Other.Properties);
		}

		VulkanPhysicalDevice& VulkanPhysicalDevice::operator=(VulkanPhysicalDevice&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(Properties, Other.Properties);
			return *this;
		}

		const RenderInterface::DeviceProperties& VulkanPhysicalDevice::GetProperties() const
		{
			return Properties;
		}

		VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice InDevice) : Device(InDevice), Properties(GetPhysicalDeviceProperties(InDevice))
		{
		}
	}
}
