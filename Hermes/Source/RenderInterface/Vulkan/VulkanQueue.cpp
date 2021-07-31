#include "VulkanQueue.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanQueue::VulkanQueue(VkDevice InDevice, uint32 InQueueFamilyIndex)
			: Device(InDevice)
			, QueueFamilyIndex(InQueueFamilyIndex)
		{
			vkGetDeviceQueue(Device, QueueFamilyIndex, 0, &Queue);
		}

		VulkanQueue::VulkanQueue(VulkanQueue&& Other)
		{
			std::swap(Queue, Other.Queue);
		}

		VulkanQueue& VulkanQueue::operator=(VulkanQueue&& Other)
		{
			std::swap(Queue, Other.Queue);
			return *this;
		}
	}
}
