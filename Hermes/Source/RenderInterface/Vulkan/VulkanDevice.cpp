#include "VulkanDevice.h"

#include "RenderInterface/Vulkan/VulkanSwapchain.h"
#include "Core/Application/GameLoop.h"
#include "RenderInterface/Vulkan/VulkanQueue.h"
#include "RenderInterface/Vulkan/VulkanResource.h"
#include "Platform/GenericPlatform/PlatformMisc.h"

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

			std::vector<const char*> EnabledExtensions;
			std::vector<VkExtensionProperties> AvailableExtensions;
			uint32 AvailableExtensionsCount;
			vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &AvailableExtensionsCount, nullptr);
			AvailableExtensions.resize(AvailableExtensionsCount);
			vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &AvailableExtensionsCount, AvailableExtensions.data());

			bool SwapchainExtensionSupported = false;
			bool MemoryBudgetExtensionSupported = false;
			for (const auto& Extension : AvailableExtensions)
			{
				if (strcmp(Extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
				{
					SwapchainExtensionSupported = true;
					EnabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
				}
				if (strcmp(Extension.extensionName, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0)
				{
					MemoryBudgetExtensionSupported = true;
					EnabledExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
				}
			}

			if (!SwapchainExtensionSupported)
			{
				PlatformMisc::ExitWithMessageBox(-1, L"Vulkan error", L"Selected Vulkan device does not support VK_KHR_swapchain extension. Update your GPU driver and try again.");
			}
			HERMES_LOG_INFO(L"VK_EXT_memory_budget extension support: %s", MemoryBudgetExtensionSupported ? L"true" : L"false");
			
			VkDeviceCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			CreateInfo.queueCreateInfoCount = (uint32)QueueCreateInfos.size();
			CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
			CreateInfo.ppEnabledExtensionNames = EnabledExtensions.data();
			CreateInfo.enabledExtensionCount = (uint32)EnabledExtensions.size();
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
			
			VkBool32 IsPresentationSupported = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, RenderQueueIndex, Surface, &IsPresentationSupported);
			if (!IsPresentationSupported)
			{
				HERMES_LOG_FATAL(L"Selected render queue does not support presentation.");
				GGameLoop->RequestExit();
				return;
			}

			VmaAllocatorCreateInfo AllocatorCreateInfo = {};
			if (MemoryBudgetExtensionSupported)
				AllocatorCreateInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
			AllocatorCreateInfo.physicalDevice = PhysicalDevice;
			AllocatorCreateInfo.device = Device;
			AllocatorCreateInfo.pAllocationCallbacks = GVulkanAllocator;
			AllocatorCreateInfo.instance = Instance;
			AllocatorCreateInfo.vulkanApiVersion = GVulkanVersion;
			VK_CHECK_RESULT(vmaCreateAllocator(&AllocatorCreateInfo, &Allocator));
		}

		VulkanDevice::~VulkanDevice()
		{
			vmaDestroyAllocator(Allocator);
			vkDestroyDevice(Device, GVulkanAllocator);
		}

		VulkanDevice::VulkanDevice(VulkanDevice&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(PhysicalDevice, Other.PhysicalDevice);
			std::swap(Instance, Other.Instance);
			std::swap(Surface, Other.Surface);
			std::swap(PresentationQueue, Other.PresentationQueue);
			std::swap(Allocator, Other.Allocator);
		}

		VulkanDevice& VulkanDevice::operator=(VulkanDevice&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(PhysicalDevice, Other.PhysicalDevice);
			std::swap(Instance, Other.Instance);
			std::swap(Surface, Other.Surface);
			std::swap(PresentationQueue, Other.PresentationQueue);
			std::swap(Allocator, Other.Allocator);
			return *this;
		}

		std::shared_ptr<RenderInterface::Swapchain> VulkanDevice::CreateSwapchain(Vec2i Size, uint32 NumFrames)
		{
			return std::make_shared<VulkanSwapchain>(PhysicalDevice, Device, Surface, Size, NumFrames);
		}

		std::shared_ptr<RenderInterface::Queue> VulkanDevice::GetQueue(RenderInterface::QueueType Type)
		{
			// NOTE : we need to 'lazily construct' those because we can't use shared_from_this() in constructor
			switch (Type)
			{
			case RenderInterface::QueueType::Render:
				if (RenderQueue == nullptr)
					RenderQueue = std::make_shared<VulkanQueue>(shared_from_this(), RenderQueueIndex);
				return RenderQueue;
				break;
			case RenderInterface::QueueType::Transfer:
				if (TransferQueue == nullptr)
					TransferQueue = std::make_shared<VulkanQueue>(shared_from_this(), TransferQueueIndex);
				return TransferQueue;
				break;
			case RenderInterface::QueueType::Presentation:
				if (PresentationQueue == nullptr)
					PresentationQueue = RenderQueue;
				return PresentationQueue;
				break;
			default:
				HERMES_LOG_FATAL(L"Someone is trying to get unknown queue type from logical device.");
				GGameLoop->RequestExit();
				return {};
			}
		}

		std::shared_ptr<RenderInterface::Resource> VulkanDevice::CreateBuffer(size_t Size, RenderInterface::ResourceUsageType Usage)
		{
			return std::make_shared<VulkanResource>(shared_from_this(), Size, Usage);
		}

		void VulkanDevice::WaitForIdle()
		{
			vkDeviceWaitIdle(Device);
		}
	}
}
