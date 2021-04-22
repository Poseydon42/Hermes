#include "VulkanDevice.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanDevice::VulkanDevice(VkPhysicalDevice PhysicalDevice, const std::vector<RenderInterface::QueueFamilyProperties>& Queues)
		{
			std::vector<VkDeviceQueueCreateInfo> QueueCreateInfo;
			QueueCreateInfo.reserve(Queues.size());
			uint32 MaxCount = 0;
			for (const auto& Queue : Queues)
				MaxCount = (Queue.Count > MaxCount ? Queue.Count : MaxCount);
			std::vector<float> Priorities(MaxCount, 1.0f);
			for (const auto& Queue : Queues)
			{
				VkDeviceQueueCreateInfo Info = {};
				Info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				Info.pQueuePriorities = Priorities.data();
				Info.queueCount = Queue.Count;
				Info.queueFamilyIndex = Queue.Index;
				QueueCreateInfo.push_back(Info);
			}
			
			VkDeviceCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			CreateInfo.queueCreateInfoCount = (uint32)QueueCreateInfo.size();
			CreateInfo.pQueueCreateInfos = QueueCreateInfo.data();
			const auto* SwapchainExtensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			CreateInfo.enabledExtensionCount = 1;
			CreateInfo.ppEnabledExtensionNames = &SwapchainExtensionName;
			VkPhysicalDeviceFeatures RequiredFeatures = {};
			CreateInfo.pEnabledFeatures = &RequiredFeatures;
			VK_CHECK_RESULT(vkCreateDevice(PhysicalDevice, &CreateInfo, GVulkanAllocator, &Device))
		}

		VulkanDevice::~VulkanDevice()
		{
			vkDestroyDevice(Device, GVulkanAllocator);
		}

		VulkanDevice::VulkanDevice(VulkanDevice&& Other)
		{
			std::swap(Device, Other.Device);
		}

		VulkanDevice& VulkanDevice::operator=(VulkanDevice&& Other)
		{
			std::swap(Device, Other.Device);
			return *this;
		}
	}
}
