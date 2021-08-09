#pragma once

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanQueue;
		
		class HERMES_API VulkanDevice : public RenderInterface::Device, public std::enable_shared_from_this<VulkanDevice>
		{
			MAKE_NON_COPYABLE(VulkanDevice)
		public:
			VulkanDevice(VkPhysicalDevice InPhysicalDevice, VkInstance InInstance, VkSurfaceKHR InSurface);
			
			~VulkanDevice() override;
			VulkanDevice(VulkanDevice&& Other);
			VulkanDevice& operator=(VulkanDevice&& Other);

			std::shared_ptr<RenderInterface::Swapchain> CreateSwapchain(Vec2i Size, uint32 NumFrames) override;

			std::shared_ptr<RenderInterface::Queue> GetQueue(RenderInterface::QueueType Type) override;
			
			std::shared_ptr<RenderInterface::Resource> CreateBuffer(size_t Size, RenderInterface::ResourceUsageType Usage) override;
			
			void WaitForIdle() override;

			// Don't make const - device 'state' includes allocations, thus it can be changed through VmaAllocator instance
			VmaAllocator GetAllocator() { return Allocator; }
			VkDevice GetDevice() { return Device; }
		private:
			VkDevice Device;
			VkPhysicalDevice PhysicalDevice;
			VkInstance Instance;
			VkSurfaceKHR Surface;
			VmaAllocator Allocator;

			int32 RenderQueueIndex = -1, TransferQueueIndex = -1;

			std::shared_ptr<RenderInterface::Queue> RenderQueue;
			std::shared_ptr<RenderInterface::Queue> TransferQueue;
			std::shared_ptr<RenderInterface::Queue> PresentationQueue;
		};
	}
}
