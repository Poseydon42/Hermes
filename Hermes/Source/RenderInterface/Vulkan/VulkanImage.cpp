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

		VulkanImage::VulkanImage(VulkanImage&& Other)
		{
			*this = std::move(Other);
		}

		VulkanImage& VulkanImage::operator=(VulkanImage&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(Handle, Other.Handle);
			std::swap(Size, Other.Size);
			std::swap(Format, Other.Format);
			std::swap(IsOwned, Other.IsOwned);
			
			return *this;
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
