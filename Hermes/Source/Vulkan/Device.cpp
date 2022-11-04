#include "Device.h"

#include "Platform/GenericPlatform/PlatformMisc.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Descriptor.h"
#include "Vulkan/Fence.h"
#include "Vulkan/Framebuffer.h"
#include "Vulkan/Image.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Queue.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/Sampler.h"
#include "Vulkan/Shader.h"
#include "Vulkan/Swapchain.h"

namespace Hermes::Vulkan
{
	Device::VkDeviceHolder::~VkDeviceHolder()
	{
		vkDeviceWaitIdle(Device);
		vmaDestroyAllocator(Allocator);
		vkDestroyDevice(Device, GVulkanAllocator);
	}

	Device::Device(std::shared_ptr<Instance::VkInstanceHolder> InInstance, VkPhysicalDevice InPhysicalDevice,
	               const IPlatformWindow& InWindow)
		: Holder(std::make_shared<VkDeviceHolder>())
		, Window(InWindow)
	{
		Holder->Instance = std::move(InInstance);
		Holder->PhysicalDevice = InPhysicalDevice;

		std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;

		uint32 QueueCount = 0;
		std::vector<VkQueueFamilyProperties> QueueFamilies;
		vkGetPhysicalDeviceQueueFamilyProperties(Holder->PhysicalDevice, &QueueCount, nullptr);
		QueueFamilies.resize(QueueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(Holder->PhysicalDevice, &QueueCount, QueueFamilies.data());
		float QueuePriority = 1.0f;

		int32 GraphicsQueueFamilyIndex = -1, TransferQueueFamilyIndex = -1;
		for (size_t Index = 0; Index < QueueFamilies.size(); Index++)
		{
			const auto& QueueFamily = QueueFamilies[Index];
			bool QueueUsed = false;
			if (GraphicsQueueFamilyIndex == -1 && (QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) ==
				VK_QUEUE_GRAPHICS_BIT && !QueueUsed)
			{
				GraphicsQueueFamilyIndex = static_cast<int32>(Index);
				QueueUsed = true;
			}
			if (TransferQueueFamilyIndex == -1 && (QueueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) ==
				VK_QUEUE_TRANSFER_BIT && !QueueUsed)
			{
				TransferQueueFamilyIndex = static_cast<int32>(Index);
				QueueUsed = true;
			}

			if (!QueueUsed)
				continue;

			VkDeviceQueueCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			CreateInfo.queueCount = 1;
			CreateInfo.queueFamilyIndex = static_cast<uint32>(Index);
			CreateInfo.pQueuePriorities = &QueuePriority;
			QueueCreateInfos.push_back(CreateInfo);
		}

		std::vector<const char*> EnabledExtensions;
		std::vector<VkExtensionProperties> AvailableExtensions;
		uint32 AvailableExtensionsCount;
		vkEnumerateDeviceExtensionProperties(Holder->PhysicalDevice, nullptr, &AvailableExtensionsCount, nullptr);
		AvailableExtensions.resize(AvailableExtensionsCount);
		vkEnumerateDeviceExtensionProperties(Holder->PhysicalDevice, nullptr, &AvailableExtensionsCount,
		                                     AvailableExtensions.data());

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
			PlatformMisc::ExitWithMessageBox(1, L"Vulkan error",
			                                 L"Selected Vulkan device does not support VK_KHR_swapchain extension. Update your GPU driver and try again.");
		}
		HERMES_LOG_INFO(L"VK_EXT_memory_budget extension support: %s",
		                MemoryBudgetExtensionSupported ? L"true" : L"false");

		VkDeviceCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		CreateInfo.queueCreateInfoCount = static_cast<uint32>(QueueCreateInfos.size());
		CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
		CreateInfo.ppEnabledExtensionNames = EnabledExtensions.data();
		CreateInfo.enabledExtensionCount = static_cast<uint32>(EnabledExtensions.size());

		bool IsAnisotropyAvailable = false;
		VkPhysicalDeviceFeatures AvailableFeatures;
		vkGetPhysicalDeviceFeatures(Holder->PhysicalDevice, &AvailableFeatures);
		IsAnisotropyAvailable = AvailableFeatures.samplerAnisotropy;

		VkPhysicalDeviceFeatures RequiredFeatures = {};
		RequiredFeatures.samplerAnisotropy = IsAnisotropyAvailable;
		CreateInfo.pEnabledFeatures = &RequiredFeatures;
		VK_CHECK_RESULT(vkCreateDevice(Holder->PhysicalDevice, &CreateInfo, GVulkanAllocator, &Holder->Device));

		if (TransferQueueFamilyIndex == -1)
		{
			// We have to use render queue to perform transfer operations then
			TransferQueueFamilyIndex = GraphicsQueueFamilyIndex;
		}
		if (GraphicsQueueFamilyIndex == -1)
		{
			PlatformMisc::ExitWithMessageBox(1, L"Vulkan error",
			                                 L"Cannot find any render queue on the selected Vulkan device.");
		}

		VkBool32 IsPresentationSupported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(Holder->PhysicalDevice, GraphicsQueueFamilyIndex,
		                                     Holder->Instance->Surface,
		                                     &IsPresentationSupported);
		if (!IsPresentationSupported)
		{
			PlatformMisc::ExitWithMessageBox(1, L"Vulkan error", L"Render queue does not support presentation.");
		}

		VmaAllocatorCreateInfo AllocatorCreateInfo = {};
		if (MemoryBudgetExtensionSupported)
			AllocatorCreateInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		AllocatorCreateInfo.physicalDevice = Holder->PhysicalDevice;
		AllocatorCreateInfo.device = Holder->Device;
		AllocatorCreateInfo.pAllocationCallbacks = GVulkanAllocator;
		AllocatorCreateInfo.instance = Holder->Instance->Instance;
		AllocatorCreateInfo.vulkanApiVersion = GVulkanVersion;
		VK_CHECK_RESULT(vmaCreateAllocator(&AllocatorCreateInfo, &Holder->Allocator));

		GraphicsQueue = std::make_unique<Queue>(Holder, GraphicsQueueFamilyIndex);
		TransferQueue = std::make_unique<Queue>(Holder, TransferQueueFamilyIndex);
		// NOTE: render queue must always have presentation capabilities
		vkGetDeviceQueue(Holder->Device, GraphicsQueueFamilyIndex, 0, &Holder->PresentationQueue);
	}

	// NOTE: we need to explicitly declare a destructor instead of doing '= default' to avoid circular dependency
	//       with Queue and Queue.h
	Device::~Device()
	{
	}

	std::unique_ptr<Swapchain> Device::CreateSwapchain(uint32 NumFrames) const
	{
		return std::make_unique<Swapchain>(Holder, Window, NumFrames);
	}

	const Queue& Device::GetQueue(VkQueueFlags Type) const
	{
		// Only graphics and transfer queues are supported at the moment
		HERMES_ASSERT((Type & VK_QUEUE_GRAPHICS_BIT) || (Type & VK_QUEUE_TRANSFER_BIT));
		// Check that only a single bit flag in the queue type is set since it is not required for
		// a VkDevice to have a queue that has more than 1 type (e.g. graphics+transfer queue is not
		// necessary present.
		HERMES_ASSERT((Type & (Type - 1)) == 0);
		if (Type & VK_QUEUE_GRAPHICS_BIT)
			return *GraphicsQueue;
		// Transfer queue is the only option left
		return *TransferQueue;
	}

	std::unique_ptr<Buffer> Device::CreateBuffer(size_t Size, VkBufferUsageFlags Usage,
	                                             bool IsMappable/* = false*/) const
	{
		return std::make_unique<Buffer>(Holder, Size, Usage, IsMappable);
	}

	std::unique_ptr<Fence> Device::CreateFence(bool InitialState/* = false */) const
	{
		return std::make_unique<Fence>(Holder, InitialState);
	}

	std::unique_ptr<Shader> Device::CreateShader(const String& Path, VkShaderStageFlagBits Type) const
	{
		return std::make_unique<Shader>(Holder, Path, Type);
	}

	std::unique_ptr<RenderPass> Device::CreateRenderPass(
		const std::vector<std::pair<VkAttachmentDescription, AttachmentType>>& Attachments) const
	{
		return std::make_unique<RenderPass>(Holder, Attachments);
	}

	std::unique_ptr<Pipeline> Device::CreatePipeline(const RenderPass& RenderPass,
	                                                 const PipelineDescription& Description) const
	{
		return std::make_unique<Pipeline>(Holder, RenderPass, Description);
	}

	std::unique_ptr<Framebuffer> Device::CreateFramebuffer(const RenderPass& RenderPass,
	                                                       const std::vector<const ImageView*>& Attachments,
	                                                       Vec2ui Dimensions) const
	{
		return std::make_unique<Framebuffer>(Holder, RenderPass, Attachments, Dimensions);
	}

	std::unique_ptr<DescriptorSetLayout> Device::CreateDescriptorSetLayout(
		const std::vector<VkDescriptorSetLayoutBinding>& Bindings) const
	{
		return std::make_unique<DescriptorSetLayout>(Holder, Bindings);
	}

	std::unique_ptr<DescriptorSetPool> Device::CreateDescriptorSetPool(uint32 NumberOfSets,
	                                                                   const std::vector<VkDescriptorPoolSize>&
	                                                                   Subpools,
	                                                                   bool SupportIndividualDeallocations /*= false*/)
	const
	{
		return std::make_unique<DescriptorSetPool>(Holder, NumberOfSets, Subpools, SupportIndividualDeallocations);
	}

	std::unique_ptr<Sampler> Device::CreateSampler(const SamplerDescription& Description) const
	{
		return std::make_unique<Sampler>(Holder, Description);
	}

	std::unique_ptr<Image> Device::CreateImage(Vec2ui Dimensions, VkImageUsageFlags Usage, VkFormat Format,
	                                           uint32 MipLevels) const
	{
		return std::make_unique<Image>(Holder, Dimensions, Usage, Format, MipLevels, false);
	}

	std::unique_ptr<Image> Device::CreateCubemap(Vec2ui Dimensions, VkImageUsageFlags Usage, VkFormat Format,
	                                             uint32 MipLevels) const
	{
		return std::make_unique<Image>(Holder, Dimensions, Usage, Format, MipLevels, true);
	}

	void Device::WaitForIdle() const
	{
		vkDeviceWaitIdle(Holder->Device);
	}
}
