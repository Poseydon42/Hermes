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
			, DefaultView(VK_NULL_HANDLE)
			, IsOwned(false)
		{
			CreateDefaultView();
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
			std::swap(DefaultView, Other.DefaultView);
			
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

		VkImageView VulkanImage::GetDefaultView() const
		{
			return DefaultView;
		}

		void VulkanImage::CreateDefaultView()
		{
			VkImageViewCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			CreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			CreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			CreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			CreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			CreateInfo.format = Format;
			CreateInfo.image = Handle;
			CreateInfo.subresourceRange.aspectMask = VkAspectFlagsFromVkFormat(Format);
			CreateInfo.subresourceRange.baseArrayLayer = 0;
			CreateInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
			CreateInfo.subresourceRange.baseMipLevel = 0;
			CreateInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
			CreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

			VK_CHECK_RESULT(vkCreateImageView(Device->GetDevice(), &CreateInfo, GVulkanAllocator, &DefaultView))
		}
	}
}
