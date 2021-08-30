#include "VulkanImage.h"

#include "RenderInterface/Vulkan/VulkanDevice.h"
#include "RenderInterface/Vulkan/VulkanCommonTypes.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanImage::VulkanImage(std::shared_ptr<VulkanDevice> InDevice, VkImage InImage, VkFormat InFormat, Vec2ui InSize)
			: Device(std::move(InDevice))
			, Handle(InImage)
			, Size(InSize)
			, Format(InFormat)
			, IsOwned(false)
		{
		}

		VulkanImage::~VulkanImage()
		{
			if (IsOwned)
				vkDestroyImage(Device->GetDevice(), Handle, GVulkanAllocator);
		}

		Vec2ui VulkanImage::GetSize() const
		{
			return Size;
		}

		RenderInterface::DataFormat VulkanImage::GetDataFormat() const
		{
			return VkFormatToDataFormat(Format);
		}

		VkImage VulkanImage::GetImage() const
		{
			return Handle;
		}
	}
}
