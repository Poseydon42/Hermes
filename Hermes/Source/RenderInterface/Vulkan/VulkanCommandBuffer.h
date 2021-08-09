#pragma once

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/CommandBuffer.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanDevice;

		class VulkanCommandBuffer : public RenderInterface::CommandBuffer
		{
		public:
			MAKE_NON_COPYABLE(VulkanCommandBuffer)

			VulkanCommandBuffer(std::shared_ptr<VulkanDevice> InDevice, VkCommandPool InPool, bool IsPrimaryBuffer);

			~VulkanCommandBuffer() override;

			VulkanCommandBuffer(VulkanCommandBuffer&& Other);
			VulkanCommandBuffer& operator=(VulkanCommandBuffer&& Other);

			void BeginRecording() override;
			
			void EndRecording() override;
			
			void CopyBuffer(const std::shared_ptr<RenderInterface::Resource>& Source,
				const std::shared_ptr<RenderInterface::Resource>& Destination,
				std::vector<RenderInterface::BufferCopyRegion> CopyRegions) override;

			VkCommandBuffer GetBuffer() { return Buffer; }
		private:
			VkCommandBuffer Buffer;
			VkCommandPool Pool;
			std::shared_ptr<VulkanDevice> Device;
		};
	}
}
