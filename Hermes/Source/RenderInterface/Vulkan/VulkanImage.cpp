#include "VulkanImage.h"

#include "RenderInterface/Vulkan/VulkanDevice.h"
#include "RenderInterface/Vulkan/VulkanCommonTypes.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanImage::VulkanImage(std::shared_ptr<const VulkanDevice> InDevice, VkImage InImage, VkFormat InFormat,
		                         Vec2ui InSize, RenderInterface::ImageUsageType InUsage, bool InIsCubemapCompatible)
			: Device(std::move(InDevice))
			, Handle(InImage)
			, Size(InSize)
			, Format(InFormat)
			, DefaultView(VK_NULL_HANDLE)
			, Allocation(VK_NULL_HANDLE)
			, IsOwned(false)
			, MipLevelCount(1)
			, IsCubemapCompatible(InIsCubemapCompatible)
			, Usage(InUsage)
		{
		}

		VulkanImage::VulkanImage(std::shared_ptr<const VulkanDevice> InDevice, Vec2ui InSize,
			RenderInterface::ImageUsageType InUsage, RenderInterface::DataFormat InFormat,
			uint32 InMipLevels, RenderInterface::ImageLayout InitialLayout,
			bool InIsCubemapCompatible)
			: Device(std::move(InDevice))
			, Handle(VK_NULL_HANDLE)
			, Size(InSize)
			, Format(DataFormatToVkFormat(InFormat))
			, DefaultView(VK_NULL_HANDLE)
			, Allocation(VK_NULL_HANDLE)
			, IsOwned(false)
			, MipLevelCount(InMipLevels)
			, IsCubemapCompatible(InIsCubemapCompatible)
			, Usage(InUsage)
		{
			VkImageCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			CreateInfo.extent.width = Size.X;
			CreateInfo.extent.height = Size.Y;
			CreateInfo.extent.depth = 1;
			CreateInfo.arrayLayers = IsCubemapCompatible ? 6 : 1;
			CreateInfo.format = Format;
			CreateInfo.initialLayout = ImageLayoutToVkImageLayout(InitialLayout);
			CreateInfo.imageType = VK_IMAGE_TYPE_2D;
			CreateInfo.mipLevels = InMipLevels;
			CreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			CreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // TODO
			CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			if (IsCubemapCompatible)
				CreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

			if (static_cast<bool>(InUsage & RenderInterface::ImageUsageType::Sampled))
				CreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
			if (static_cast<bool>(InUsage & RenderInterface::ImageUsageType::ColorAttachment))
				CreateInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			if (static_cast<bool>(InUsage & RenderInterface::ImageUsageType::DepthStencilAttachment))
				CreateInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			if (static_cast<bool>(InUsage & RenderInterface::ImageUsageType::InputAttachment))
				CreateInfo.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
			if (static_cast<bool>(InUsage & RenderInterface::ImageUsageType::CopySource))
				CreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			if (static_cast<bool>(InUsage & RenderInterface::ImageUsageType::CopyDestination))
				CreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

			VmaAllocationCreateInfo AllocationInfo = {};
			if (static_cast<bool>(InUsage & RenderInterface::ImageUsageType::CPUAccessible))
				AllocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			else
				AllocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			VK_CHECK_RESULT(vmaCreateImage(Device->GetAllocator(), &CreateInfo, &AllocationInfo, &Handle, &Allocation, nullptr))

			IsOwned = true;
		}

		VulkanImage::~VulkanImage()
		{
			if (IsOwned)
			{
				vmaDestroyImage(Device->GetAllocator(), Handle, Allocation);
			}
			vkDestroyImageView(Device->GetDevice(), DefaultView, GVulkanAllocator);
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
			std::swap(Allocation, Other.Allocation);
			std::swap(MipLevelCount, Other.MipLevelCount);
			std::swap(IsCubemapCompatible, Other.IsCubemapCompatible);
			std::swap(Usage, Other.Usage);
			
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

		uint32 VulkanImage::GetMipLevelsCount() const
		{
			return MipLevelCount;
		}

		RenderInterface::ImageUsageType VulkanImage::GetUsageFlags() const
		{
			return Usage;
		}

		VkImage VulkanImage::GetImage() const
		{
			return Handle;
		}

		VkImageView VulkanImage::GetDefaultView() const
		{
			if (DefaultView == VK_NULL_HANDLE)
				CreateDefaultView();
			return DefaultView;
		}

		bool VulkanImage::GetIsCubemapCompatible() const
		{
			return IsCubemapCompatible;
		}

		void VulkanImage::CreateDefaultView() const
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
			CreateInfo.viewType = IsCubemapCompatible ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;

			VK_CHECK_RESULT(vkCreateImageView(Device->GetDevice(), &CreateInfo, GVulkanAllocator, &DefaultView))
		}
	}
}
