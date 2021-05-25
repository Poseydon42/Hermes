#include "VulkanDevice.h"

#include "RenderInterface/Vulkan/VulkanSwapchain.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanDevice::VulkanDevice(VkPhysicalDevice InPhysicalDevice, VkInstance InInstance, VkSurfaceKHR InSurface, const std::vector<RenderInterface::QueueFamilyProperties>& Queues) :
			Device(VK_NULL_HANDLE), PhysicalDevice(InPhysicalDevice), Instance(InInstance), Surface(InSurface), PresentationQueue(VK_NULL_HANDLE)
		{
			std::vector<VkDeviceQueueCreateInfo> QueueCreateInfo;
			QueueCreateInfo.reserve(Queues.size());
			uint32 MaxCount = 0;
			for (const auto& Queue : Queues)
				MaxCount = (Queue.Count > MaxCount ? Queue.Count : MaxCount);
			std::vector<float> Priorities(MaxCount, 1.0f);
			bool PresentationQueueFound = false;
			uint32 PresentationFamilyIndex = (uint32)-1;
			// We check whether there is at least one queue family that supports presentation
			// among those that were requested by user
			for (const auto& Queue : Queues)
			{
				VkDeviceQueueCreateInfo Info = {};
				Info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				Info.pQueuePriorities = Priorities.data();
				Info.queueCount = Queue.Count;
				Info.queueFamilyIndex = Queue.Index;
				QueueCreateInfo.push_back(Info);
				VkBool32 PresentationSupported = false;
				VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, Queue.Index, Surface, &PresentationSupported));
				if (PresentationSupported)
				{
					PresentationQueueFound = true;
					PresentationFamilyIndex = Queue.Index;
				}
			}
			// If we haven't found such queue family among those that were requested
			// then we loop through all available ones and select first suitable
			if (!PresentationQueueFound)
			{
				uint32 QueueFamilyCount;
				std::vector<VkQueueFamilyProperties> QueueFamilies;
				vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, VK_NULL_HANDLE);
				HERMES_ASSERT(QueueFamilyCount);
				QueueFamilies.resize(QueueFamilyCount);
				vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, QueueFamilies.data());
				for (uint32 Index = 0; Index < QueueFamilyCount; Index++)
				{
					VkBool32 PresentationSupported = false;
					VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, Index, Surface, &PresentationSupported));
					if (PresentationSupported)
					{
						VkDeviceQueueCreateInfo PresentationQueueCreateInfo = {};
						PresentationQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
						PresentationQueueCreateInfo.pQueuePriorities = Priorities.data();
						PresentationQueueCreateInfo.queueCount = 1;
						PresentationQueueCreateInfo.queueFamilyIndex = Index;
						QueueCreateInfo.push_back(PresentationQueueCreateInfo);
						PresentationQueueFound = true;
						PresentationFamilyIndex = Index;
					}
				}
			}
			HERMES_ASSERT_LOG(PresentationQueueFound, L"Failed to find family queue that supports presentaion operations.");
			
			VkDeviceCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			CreateInfo.queueCreateInfoCount = (uint32)QueueCreateInfo.size();
			CreateInfo.pQueueCreateInfos = QueueCreateInfo.data();
			const auto* SwapchainExtensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			CreateInfo.enabledExtensionCount = 1;
			CreateInfo.ppEnabledExtensionNames = &SwapchainExtensionName;
			VkPhysicalDeviceFeatures RequiredFeatures = {};
			CreateInfo.pEnabledFeatures = &RequiredFeatures;
			VK_CHECK_RESULT(vkCreateDevice(PhysicalDevice, &CreateInfo, GVulkanAllocator, &Device));

			vkGetDeviceQueue(Device, PresentationFamilyIndex, 0, &PresentationQueue);
		}

		VulkanDevice::~VulkanDevice()
		{
			vkDestroyDevice(Device, GVulkanAllocator);
		}

		VulkanDevice::VulkanDevice(VulkanDevice&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(PhysicalDevice, Other.PhysicalDevice);
			std::swap(Instance, Other.Instance);
			std::swap(Surface, Other.Surface);
			std::swap(PresentationQueue, Other.PresentationQueue);
		}

		VulkanDevice& VulkanDevice::operator=(VulkanDevice&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(PhysicalDevice, Other.PhysicalDevice);
			std::swap(Instance, Other.Instance);
			std::swap(Surface, Other.Surface);
			std::swap(PresentationQueue, Other.PresentationQueue);
			return *this;
		}

		std::shared_ptr<RenderInterface::Swapchain> VulkanDevice::CreateSwapchain(Vec2i Size, uint32 NumFrames)
		{
			return std::make_shared<VulkanSwapchain>(PhysicalDevice, Device, Surface, Size, NumFrames);
		}
	}
}
