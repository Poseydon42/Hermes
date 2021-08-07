﻿#pragma once

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanQueue;
		
		class HERMES_API VulkanDevice : public RenderInterface::Device
		{
			MAKE_NON_COPYABLE(VulkanDevice)
		public:
			VulkanDevice(VkPhysicalDevice InPhysicalDevice, VkInstance InInstance, VkSurfaceKHR InSurface);
			
			~VulkanDevice() override;
			VulkanDevice(VulkanDevice&& Other);
			VulkanDevice& operator=(VulkanDevice&& Other);

			std::shared_ptr<RenderInterface::Swapchain> CreateSwapchain(Vec2i Size, uint32 NumFrames) override;

			std::shared_ptr<RenderInterface::Queue> GetQueue(RenderInterface::QueueType Type) override;
		private:
			VkDevice Device;
			VkPhysicalDevice PhysicalDevice;
			VkInstance Instance;
			VkSurfaceKHR Surface;
			VmaAllocator Allocator;

			std::shared_ptr<RenderInterface::Queue> RenderQueue;
			std::shared_ptr<RenderInterface::Queue> TransferQueue;
			std::shared_ptr<RenderInterface::Queue> PresentationQueue;
		};
	}
}
