#pragma once

#include "Core/Core.h"
#include "RenderInterface/Vulkan/Vulkan.h"
#include "RenderInterface/GenericRenderInterface/Queue.h"

namespace Hermes
{
	namespace Vulkan
	{
		class HERMES_API VulkanQueue : public RenderInterface::Queue
		{
		public:
			MAKE_NON_COPYABLE(VulkanQueue);
			
			VulkanQueue(VkDevice InDevice, uint32 InQueueFamilyIndex);

			~VulkanQueue() override = default;
			VulkanQueue(VulkanQueue&& Other);
			VulkanQueue& operator=(VulkanQueue&& Other);
		private:
			VkDevice Device;
			VkQueue Queue;
			uint32 QueueFamilyIndex;
		};
	}
}
