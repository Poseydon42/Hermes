#include "VulkanDevice.h"

#include "Core/Application/GameLoop.h"
#include "RenderInterface/Vulkan/VulkanInstance.h"
#include "RenderInterface/Vulkan/VulkanPipeline.h"
#include "RenderInterface/Vulkan/VulkanRenderPass.h"
#include "RenderInterface/Vulkan/VulkanRenderTarget.h"
#include "RenderInterface/Vulkan/VulkanShader.h"
#include "RenderInterface/Vulkan/VulkanSwapchain.h"
#include "RenderInterface/Vulkan/VulkanQueue.h"
#include "RenderInterface/Vulkan/VulkanBuffer.h"
#include "RenderInterface/Vulkan/VulkanFence.h"
#include "Platform/GenericPlatform/PlatformMisc.h"
#include "RenderInterface/Vulkan/VulkanDescriptor.h"
#include "RenderInterface/Vulkan/VulkanSampler.h"

namespace Hermes
{
	namespace Vulkan
	{

		VulkanDevice::VulkanDevice(VkPhysicalDevice InPhysicalDevice, std::shared_ptr<const VulkanInstance> InInstance, VkSurfaceKHR InSurface, std::weak_ptr<const IPlatformWindow> InWindow)
			: Device(VK_NULL_HANDLE)
			, PhysicalDevice(InPhysicalDevice)
			, Instance(std::move(InInstance))
			, Surface(InSurface)
			, Allocator(VK_NULL_HANDLE)
			, Window(std::move(InWindow))
			, RenderQueue(VK_NULL_HANDLE)
			, TransferQueue(VK_NULL_HANDLE)
			, PresentationQueue(VK_NULL_HANDLE)
		{
			std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;

			uint32 QueueCount = 0;
			std::vector<VkQueueFamilyProperties> QueueFamilies;
			vkGetPhysicalDeviceQueueFamilyProperties(InPhysicalDevice, &QueueCount, 0);
			QueueFamilies.resize(QueueCount);
			vkGetPhysicalDeviceQueueFamilyProperties(InPhysicalDevice, &QueueCount, QueueFamilies.data());
			float QueuePriority = 1.0f;

			for (size_t Index = 0; Index < QueueFamilies.size(); Index++)
			{
				const auto& QueueFamily = QueueFamilies[Index];
				bool QueueUsed = false;
				if (RenderQueueIndex == -1 && (QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT && !QueueUsed)
				{
					RenderQueueIndex = static_cast<int32>(Index);
					QueueUsed = true;
				}
				if (TransferQueueIndex == -1 && (QueueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT && !QueueUsed)
				{
					TransferQueueIndex = static_cast<int32>(Index);
					QueueUsed = true;
				}

				if (!QueueUsed)
					continue;

				VkDeviceQueueCreateInfo CreateInfo = {};
				CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				CreateInfo.queueCount = 1;
				CreateInfo.queueFamilyIndex = (uint32)Index;
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
				PlatformMisc::ExitWithMessageBox((uint32)-1, L"Vulkan error", L"Selected Vulkan device does not support VK_KHR_swapchain extension. Update your GPU driver and try again.");
			}
			HERMES_LOG_INFO(L"VK_EXT_memory_budget extension support: %s", MemoryBudgetExtensionSupported ? L"true" : L"false");
			
			VkDeviceCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			CreateInfo.queueCreateInfoCount = (uint32)QueueCreateInfos.size();
			CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
			CreateInfo.ppEnabledExtensionNames = EnabledExtensions.data();
			CreateInfo.enabledExtensionCount = (uint32)EnabledExtensions.size();

			bool IsAnisotropyAvailable = false;
			VkPhysicalDeviceFeatures AvailableFeatures;
			vkGetPhysicalDeviceFeatures(PhysicalDevice, &AvailableFeatures);
			IsAnisotropyAvailable = AvailableFeatures.samplerAnisotropy;

			VkPhysicalDeviceFeatures RequiredFeatures = {};
			RequiredFeatures.samplerAnisotropy = IsAnisotropyAvailable;
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
			AllocatorCreateInfo.instance = Instance->GetInstance();
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
			*this = std::move(Other);
		}

		VulkanDevice& VulkanDevice::operator=(VulkanDevice&& Other)
		{
			
			std::swap(Device, Other.Device);
			std::swap(PhysicalDevice, Other.PhysicalDevice);
			std::swap(Instance, Other.Instance);
			std::swap(Surface, Other.Surface);
			std::swap(RenderQueue, Other.RenderQueue);
			std::swap(TransferQueue, Other.TransferQueue);
			std::swap(PresentationQueue, Other.PresentationQueue);
			std::swap(Allocator, Other.Allocator);
			std::swap(Window, Other.Window);
			std::swap(RenderQueueIndex, Other.RenderQueueIndex);
			std::swap(TransferQueueIndex, Other.TransferQueueIndex);

			return *this;
		}

		std::unique_ptr<RenderInterface::Swapchain> VulkanDevice::CreateSwapchain(uint32 NumFrames) const
		{
			return std::make_unique<VulkanSwapchain>(shared_from_this(), PhysicalDevice, Surface, Window, NumFrames);
		}

		const RenderInterface::Queue& VulkanDevice::GetQueue(RenderInterface::QueueType Type) const
		{
			// NOTE : we need to 'lazily construct' those because we can't use shared_from_this() in constructor
			switch (Type)
			{
			case RenderInterface::QueueType::Render:
				if (RenderQueue == nullptr)
					RenderQueue = std::make_shared<VulkanQueue>(shared_from_this(), RenderQueueIndex);
				return *RenderQueue;
			case RenderInterface::QueueType::Transfer:
				if (TransferQueue == nullptr)
					TransferQueue = std::make_shared<VulkanQueue>(shared_from_this(), TransferQueueIndex);
				return *TransferQueue;
			case RenderInterface::QueueType::Presentation:
				if (PresentationQueue == nullptr)
					PresentationQueue = RenderQueue;
				return *PresentationQueue;
			default:
				HERMES_ASSERT_LOG(false, L"Someone is trying to get unknown queue type from logical device.");
			}
			return *RenderQueue;
		}

		std::unique_ptr<RenderInterface::Buffer> VulkanDevice::CreateBuffer(size_t Size, RenderInterface::BufferUsageType Usage)  const
		{
			return std::make_unique<VulkanBuffer>(shared_from_this(), Size, Usage);
		}

		std::unique_ptr<RenderInterface::Fence> VulkanDevice::CreateFence(bool InitialState) const
		{
			return std::make_unique<VulkanFence>(shared_from_this(), InitialState);
		}

		std::unique_ptr<RenderInterface::Shader> VulkanDevice::CreateShader(const String& Path, RenderInterface::ShaderType Type) const
		{
			return std::make_unique<VulkanShader>(shared_from_this(), Path, Type);
		}

		std::unique_ptr<RenderInterface::RenderPass> VulkanDevice::CreateRenderPass(const std::vector<RenderInterface::RenderPassAttachment>& Attachments) const
		{
			return std::make_unique<VulkanRenderPass>(shared_from_this(), Attachments);
		}

		std::unique_ptr<RenderInterface::Pipeline> VulkanDevice::CreatePipeline(const RenderInterface::RenderPass& RenderPass, const RenderInterface::PipelineDescription& Description) const
		{
			return std::make_unique<VulkanPipeline>(shared_from_this(), std::move(RenderPass), Description);
		}

		std::unique_ptr<RenderInterface::RenderTarget> VulkanDevice::CreateRenderTarget(const RenderInterface::RenderPass& RenderPass, const std::vector<const RenderInterface::Image*>& Attachments, Vec2ui Size) const
		{
			return std::make_unique<VulkanRenderTarget>(shared_from_this(), RenderPass, Attachments, Size);
		}

		std::unique_ptr<RenderInterface::DescriptorSetLayout> VulkanDevice::CreateDescriptorSetLayout(const std::vector<RenderInterface::DescriptorBinding>& Bindings) const
		{
			return std::make_unique<VulkanDescriptorSetLayout>(shared_from_this(), Bindings);
		}

		std::unique_ptr<RenderInterface::DescriptorSetPool> VulkanDevice::CreateDescriptorSetPool(uint32 NumberOfSets, const std::vector<RenderInterface::SubpoolDescription>& Subpools, bool SupportIndividualDeallocations) const
		{
			return std::make_unique<VulkanDescriptorSetPool>(shared_from_this(), NumberOfSets, Subpools, SupportIndividualDeallocations);
		}

		std::unique_ptr<RenderInterface::Sampler> VulkanDevice::CreateSampler(const RenderInterface::SamplerDescription& Description) const
		{
			return std::make_unique<VulkanSampler>(shared_from_this(), Description);
		}

		std::unique_ptr<RenderInterface::Image> VulkanDevice::CreateImage(Vec2ui Size, RenderInterface::ImageUsageType Usage, RenderInterface::DataFormat Format, uint32 MipLevels, RenderInterface::ImageLayout InitialLayout) const
		{
			return std::make_unique<VulkanImage>(shared_from_this(), Size, Usage, Format, MipLevels, InitialLayout);
		}

		std::unique_ptr<RenderInterface::Image> VulkanDevice::CreateCubemap(Vec2ui Size,
		                                                                    RenderInterface::ImageUsageType Usage,
		                                                                    RenderInterface::DataFormat Format,
		                                                                    uint32 MipLevels,
		                                                                    RenderInterface::ImageLayout InitialLayout)	const
		{
			return std::make_unique<VulkanImage>(shared_from_this(), Size, Usage, Format, MipLevels, InitialLayout,
			                                     true);
		}

		void VulkanDevice::WaitForIdle() const
		{
			vkDeviceWaitIdle(Device);
		}
	}
}
