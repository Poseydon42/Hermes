#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "RenderInterface/GenericRenderInterface/Image.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanDevice;

		class HERMES_API VulkanImage : public RenderInterface::Image
		{
			MAKE_NON_COPYABLE(VulkanImage);

		public:
			VulkanImage(std::shared_ptr<VulkanDevice> InDevice, VkImage InImage, VkFormat InFormat, Vec2ui InSize);

			~VulkanImage();
			VulkanImage(VulkanImage&& Other);
			VulkanImage& operator=(VulkanImage&& Other);

			Vec2ui GetSize() const override;
			
			RenderInterface::DataFormat GetDataFormat() const override;

			VkImage GetImage() const;

		private:
			std::shared_ptr<VulkanDevice> Device;
			VkImage Handle;
			Vec2ui Size;
			VkFormat Format;
			bool IsOwned;
		};
	}
}
