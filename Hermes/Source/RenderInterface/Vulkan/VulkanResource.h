#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Resource.h"
#include "Vulkan.h"
#include "Math/Vector2.h"

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

			/**
			 * Non-owned image constructor
			 */
			VulkanResource(std::shared_ptr<VulkanDevice> InDevice, VkImage Image, Vec2ui Size);
			
			~VulkanResource() override;

			VulkanResource(VulkanResource&& Other);
			VulkanResource& operator=(VulkanResource&& Other);
			
			void* Map() override;
			
			void Unmap() override;

			Vec2ui GetImageSize() const override;

			VkBuffer GetAsBuffer() const;
			VkImage GetAsImage() const;
		private:
			struct Buffer
			{
				VkBuffer Handle = VK_NULL_HANDLE;
			} AsBuffer;
			
			struct Image
			{
				VkImage Handle = VK_NULL_HANDLE;
				Vec2ui Size = {};
			} AsImage;

			VmaAllocation Allocation;
			std::shared_ptr<VulkanDevice> Device;
			bool IsMapped;
			bool IsOwned;
		};
	}
}
