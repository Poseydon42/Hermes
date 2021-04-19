#pragma once

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanInstance;
		
		class HERMES_API VulkanPhysicalDevice final : public RenderInterface::PhysicalDevice
		{
			MAKE_NON_COPYABLE(VulkanPhysicalDevice)
		public:
			~VulkanPhysicalDevice() override;
			VulkanPhysicalDevice(VulkanPhysicalDevice&&);
			VulkanPhysicalDevice& operator=(VulkanPhysicalDevice&&);
		private:
			VulkanPhysicalDevice(VkPhysicalDevice InDevice);
			
			VkPhysicalDevice Device = VK_NULL_HANDLE;

			friend class VulkanInstance;
		};
	}
}
