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
			
			VulkanQueue(std::shared_ptr<const VulkanDevice> InDevice, uint32 InQueueFamilyIndex);

			~VulkanQueue() override;
			VulkanQueue(VulkanQueue&& Other);
			VulkanQueue& operator=(VulkanQueue&& Other);

			std::unique_ptr<RenderInterface::CommandBuffer> CreateCommandBuffer(bool IsPrimaryBuffer) const override;

			virtual void SubmitCommandBuffer(const RenderInterface::CommandBuffer& Buffer,
			                                 std::optional<RenderInterface::Fence*> Fence) const override;

			void WaitForIdle() const override;

			VkQueue GetQueue() const;

			uint32 GetQueueFamilyIndex() const;
			
		private:
			std::shared_ptr<const VulkanDevice> Device;
			VkQueue Queue;
			VkCommandPool CommandPool;
			uint32 QueueFamilyIndex;
		};
	}
}
