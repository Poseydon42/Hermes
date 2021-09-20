#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderInterface/Vulkan/Vulkan.h"
#include "RenderInterface/GenericRenderInterface/Queue.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanDevice;

		class HERMES_API VulkanQueue : public RenderInterface::Queue
		{
		public:
			MAKE_NON_COPYABLE(VulkanQueue);
			
			VulkanQueue(std::shared_ptr<VulkanDevice> InDevice, uint32 InQueueFamilyIndex);

			~VulkanQueue() override;
			VulkanQueue(VulkanQueue&& Other);
			VulkanQueue& operator=(VulkanQueue&& Other);

			std::shared_ptr<RenderInterface::CommandBuffer> CreateCommandBuffer(bool IsPrimaryBuffer) override;
			
			void SubmitCommandBuffer(std::shared_ptr<RenderInterface::CommandBuffer> Buffer, std::optional<std::shared_ptr<RenderInterface::Fence>> Fence) override;

			VkQueue GetQueue() const;

			uint32 GetQueueFamilyIndex() const;
		private:
			std::shared_ptr<VulkanDevice> Device;
			VkQueue Queue;
			VkCommandPool CommandPool;
			uint32 QueueFamilyIndex;
		};
	}
}
