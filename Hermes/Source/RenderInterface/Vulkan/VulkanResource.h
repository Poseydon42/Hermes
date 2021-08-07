#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Resource.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanDevice;
		
		class HERMES_API VulkanResource final : public RenderInterface::ResourceBase
		{
		public:
			MAKE_NON_COPYABLE(VulkanResource)
			
			/**
			 * Buffer constructor
			 */
			VulkanResource(std::shared_ptr<VulkanDevice> InDevice, size_t BufferSize, RenderInterface::ResourceUsageType Usage);
			
			~VulkanResource() override;

			VulkanResource(VulkanResource&& Other);
			VulkanResource& operator=(VulkanResource&& Other);
			
			void* Map() override;
			
			void Unmap() override;

		private:
			union As
			{
				struct Buffer
				{
					VkBuffer Handle;
				} Buffer;
			} As;

			VmaAllocation Allocation;
			std::shared_ptr<VulkanDevice> Device;
		};
	}
}
