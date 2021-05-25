#pragma once

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanDevice : public RenderInterface::Device
		{
			MAKE_NON_COPYABLE(VulkanDevice)
		public:
			VulkanDevice(VkPhysicalDevice InPhysicalDevice, VkInstance InInstance, VkSurfaceKHR InSurface, const std::vector<RenderInterface::QueueFamilyProperties>& Queues);
			
			~VulkanDevice() override;
			VulkanDevice(VulkanDevice&& Other);
			VulkanDevice& operator=(VulkanDevice&& Other);

			std::shared_ptr<RenderInterface::Swapchain> CreateSwapchain(Vec2i Size, uint32 NumFrames) override;
		private:
			VkDevice Device;
			VkPhysicalDevice PhysicalDevice;
			VkInstance Instance;
			VkSurfaceKHR Surface;

			VkQueue PresentationQueue;
		};
	}
}
