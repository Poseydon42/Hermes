#include "VulkanDevice.h"

#include "RenderInterface/Vulkan/VulkanSwapchain.h"
#include "Core/Application/GameLoop.h"
#include "RenderInterface/Vulkan/VulkanQueue.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanDevice::VulkanDevice(VkPhysicalDevice InPhysicalDevice, VkInstance InInstance, VkSurfaceKHR InSurface) :
			Device(VK_NULL_HANDLE),
			PhysicalDevice(InPhysicalDevice),
			Instance(InInstance),
			Surface(InSurface),
			RenderQueue(VK_NULL_HANDLE),
			TransferQueue(VK_NULL_HANDLE),
			PresentationQueue(VK_NULL_HANDLE)
		{
			std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
			int32 RenderQueueIndex = -1, TransferQueueIndex = -1;

			uint32 QueueCount = 0;
			std::vector<VkQueueFamilyProperties> QueueFamilies;
			vkGetPhysicalDeviceQueueFamilyProperties(InPhysicalDevice, &QueueCount, 0);
			QueueFamilies.resize(QueueCount);
			vkGetPhysicalDeviceQueueFamilyProperties(InPhysicalDevice, &QueueCount, QueueFamilies.data());
			float QueuePriority = 1.0f;

			for (size_t i = 0; i < QueueFamilies.size(); i++)
			{
				const auto& QueueFamily = QueueFamilies[i];
				bool QueueUsed = false;
				if (RenderQueueIndex == -1 && (QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT && !QueueUsed)
				{
					RenderQueueIndex = (uint32)i;
					QueueUsed = true;
				}
				if (TransferQueueIndex == -1 && (QueueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT && !QueueUsed)
				{
					TransferQueueIndex = (uint32)i;
					QueueUsed = true;
				}

				if (!QueueUsed)
					continue;

				VkDeviceQueueCreateInfo CreateInfo = {};
				CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				CreateInfo.queueCount = 1;
				CreateInfo.queueFamilyIndex = (uint32)i;
				CreateInfo.pQueuePriorities = &QueuePriority;
				QueueCreateInfos.push_back(CreateInfo);
			}

			VkDeviceCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			CreateInfo.queueCreateInfoCount = (uint32)QueueCreateInfos.size();
			CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
			const auto* SwapchainExtensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			CreateInfo.enabledExtensionCount = 1;
			CreateInfo.ppEnabledExtensionNames = &SwapchainExtensionName;
			VkPhysicalDeviceFeatures RequiredFeatures = {};
			CreateInfo.pEnabledFeatures = &RequiredFeatures;
			VK_CHECK_RESULT(vkCreateDevice(PhysicalDevice, &CreateInfo, GVulkanAllocator, &Device));

			if (TransferQueueIndex == -1)
			{
				// We have to use render queue to perform transfer operations then
				TransferQueueIndex = RenderQueueIndex;
			}
			if (RenderQueueIndex == -1)
			{
				HERMES_LOG_FATAL(L"Failed to find any suitable Vulkan render queue.");
				GGameLoop->RequestExit();
				return;
			}
			RenderQueue = std::make_shared<VulkanQueue>(Device, RenderQueueIndex);
			TransferQueue = std::make_shared<VulkanQueue>(Device, TransferQueueIndex);

			VkBool32 IsPresentationSupported = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, RenderQueueIndex, Surface, &IsPresentationSupported);
			if (!IsPresentationSupported)
			{
				HERMES_LOG_FATAL(L"Selected render queue does not support presentation.");
				GGameLoop->RequestExit();
				return;
			}
			PresentationQueue = RenderQueue;
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

		std::shared_ptr<RenderInterface::Queue> VulkanDevice::GetQueue(RenderInterface::QueueType Type)
		{
			switch (Type)
			{
			case RenderInterface::QueueType::Render:
				return RenderQueue;
				break;
			case RenderInterface::QueueType::Transfer:
				return TransferQueue;
				break;
			case RenderInterface::QueueType::Presentation:
				return PresentationQueue;
				break;
			default:
				HERMES_LOG_FATAL(L"Someone is trying to get unknown queue type from logical device.");
				GGameLoop->RequestExit();
			}
		}
	}
}
