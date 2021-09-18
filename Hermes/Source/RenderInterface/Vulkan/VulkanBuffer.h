#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Buffer.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanDevice;
		
		class HERMES_API VulkanBuffer final : public RenderInterface::Buffer
		{
		public:
			MAKE_NON_COPYABLE(VulkanBuffer)
			
			VulkanBuffer(std::shared_ptr<VulkanDevice> InDevice, size_t BufferSize, RenderInterface::BufferUsageType Usage);
			
			~VulkanBuffer() override;

			VulkanBuffer(VulkanBuffer&& Other);
			VulkanBuffer& operator=(VulkanBuffer&& Other);
			
			void* Map() override;
			
			void Unmap() override;

			size_t GetSize() const override;

			VkBuffer GetBuffer() const;
			
		private:
			VkBuffer Buffer;
			VmaAllocation Allocation;
			std::shared_ptr<VulkanDevice> Device;
			bool IsMapped;
			size_t Size;
		};
	}
}
