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
			VulkanDevice(VkPhysicalDevice PhysicalDevice, const std::vector<RenderInterface::QueueFamilyProperties>& Queues);
			
			~VulkanDevice();
			VulkanDevice(VulkanDevice&& Other);
			VulkanDevice& operator=(VulkanDevice&& Other);

		private:
			VkDevice Device = VK_NULL_HANDLE;
		};
	}
}
