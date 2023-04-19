#include "Image.h"

#include <algorithm>

namespace Hermes::Vulkan
{
	uint32 CubemapSideToArrayLayer(CubemapSide Side)
	{
		switch (Side)
		{
		case CubemapSide::PositiveX:
			return 0;
		case CubemapSide::NegativeX:
			return 1;
		case CubemapSide::PositiveY:
			return 2;
		case CubemapSide::NegativeY:
			return 3;
		case CubemapSide::PositiveZ:
			return 4;
		case CubemapSide::NegativeZ:
			return 5;
		default:
			HERMES_ASSERT(false)
		}
	}

	VkImageAspectFlags FullImageAspectMask(VkFormat Format)
	{
		constexpr VkFormat FormatsWithDepthAspect[] = {
			VK_FORMAT_D16_UNORM, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT,
		};
		constexpr VkFormat FormatsWithStencilAspect[] = {
			VK_FORMAT_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT
		};

		VkImageAspectFlags Result = 0;
		if (std::ranges::find(FormatsWithDepthAspect, Format) != std::end(FormatsWithDepthAspect))
		{
			Result |= VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		if (std::ranges::find(FormatsWithStencilAspect, Format) != std::end(FormatsWithStencilAspect))
		{
			Result |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		// If the image doesn't have neither depth nor stencil aspect then it must have color aspect
		if (Result == 0)
		{
			Result = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		return Result;
	}

	Image::Image(std::shared_ptr<Device::VkDeviceHolder> InDevice, VkImage InImage, VkFormat InFormat,
	             Vec2ui InDimensions, bool InIsCubemapCompatible)
		: Holder(std::make_shared<VkImageHolder>())
		, MipLevelCount(1)
		, IsCubemapCompatible(InIsCubemapCompatible)
	{
		Holder->Device = std::move(InDevice);
		Holder->Image = InImage;
		Holder->Format = InFormat;
		Holder->Dimensions = InDimensions;
		Holder->IsOwned = false;
	}

	Image::Image(std::shared_ptr<Device::VkDeviceHolder> InDevice, Vec2ui InDimensions, VkImageUsageFlags InUsage,
	             VkFormat InFormat, uint32 InMipLevels, bool InIsCubemapCompatible)
		: Holder(std::make_shared<VkImageHolder>())
		, MipLevelCount(InMipLevels)
		, IsCubemapCompatible(InIsCubemapCompatible)
	{
		Holder->Device = std::move(InDevice);
		Holder->Format = InFormat;
		Holder->Dimensions = InDimensions;

		VkImageCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		CreateInfo.extent.width = Holder->Dimensions.X;
		CreateInfo.extent.height = Holder->Dimensions.Y;
		CreateInfo.extent.depth = 1;
		CreateInfo.arrayLayers = IsCubemapCompatible ? 6 : 1;
		CreateInfo.format = Holder->Format;
		CreateInfo.usage = InUsage;
		CreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		CreateInfo.imageType = VK_IMAGE_TYPE_2D;
		CreateInfo.mipLevels = InMipLevels;
		CreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		CreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // TODO
		CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO: perhaps we should allow the user to change this?
		// FIXME: this might have some impact on performance, we should only set this if it's reasonable to assume
		//        that the image might be used with different formats (e.g. SRGB/UNORM)
		CreateInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
		if (IsCubemapCompatible)
			CreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VmaAllocationCreateInfo AllocationInfo = {};
		AllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
		AllocationInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		VK_CHECK_RESULT(vmaCreateImage(Holder->Device->Allocator, &CreateInfo, &AllocationInfo, &Holder->Image,
			                &Holder->Allocation, nullptr))

		Holder->IsOwned = true;
	}

	std::unique_ptr<ImageView> Image::CreateImageView(const VkImageSubresourceRange& Range) const
	{
		return std::make_unique<ImageView>(Holder, Range, Holder->Format, false);
	}

	std::unique_ptr<ImageView> Image::CreateImageView(const VkImageSubresourceRange& Range, VkFormat ViewFormat) const
	{
		return std::make_unique<ImageView>(Holder, Range, ViewFormat, false);
	}

	std::unique_ptr<ImageView> Image::CreateCubemapImageView(const VkImageSubresourceRange& Range) const
	{
		HERMES_ASSERT(IsCubemapCompatible);
		return std::make_unique<ImageView>(Holder, Range, Holder->Format, true);
	}

	std::unique_ptr<ImageView> Image::CreateCubemapImageView(const VkImageSubresourceRange& Range, VkFormat ViewFormat) const
	{
		HERMES_ASSERT(IsCubemapCompatible);
		return std::make_unique<ImageView>(Holder, Range, ViewFormat, true);
	}

	std::unique_ptr<ImageView> Image::CreateDefaultImageView() const
	{
		VkImageSubresourceRange Range = {};
		Range.aspectMask = FullImageAspectMask(Holder->Format);
		Range.baseMipLevel = 0;
		Range.levelCount = MipLevelCount;
		Range.baseArrayLayer = 0;
		Range.layerCount = IsCubemapCompatible ? 6 : 1;

		return std::make_unique<ImageView>(Holder, Range, Holder->Format, IsCubemapCompatible);
	}

	VkImageSubresourceRange Image::GetFullSubresourceRange() const
	{
		VkImageSubresourceRange Result = {};
		Result.baseMipLevel = 0;
		Result.levelCount = MipLevelCount;
		Result.baseArrayLayer = 0;
		Result.layerCount = IsCubemapCompatible ? 6 : 1;
		Result.aspectMask = FullImageAspectMask(Holder->Format);

		return Result;
	}

	Vec2ui Image::GetDimensions() const
	{
		return Holder->Dimensions;
	}

	VkFormat Image::GetDataFormat() const
	{
		return Holder->Format;
	}

	VkImageAspectFlags Image::GetFullAspectMask() const
	{
		return FullImageAspectMask(Holder->Format);
	}

	uint32 Image::GetMipLevelsCount() const
	{
		return MipLevelCount;
	}

	bool Image::IsCubemap() const
	{
		return IsCubemapCompatible;
	}

	VkImage Image::GetImage() const
	{
		return Holder->Image;
	}

	Image::VkImageHolder::~VkImageHolder()
	{
		if (IsOwned)
		{
			vmaDestroyImage(Device->Allocator, Image, Allocation);
		}
	}

	ImageView::ImageView(std::shared_ptr<Image::VkImageHolder> InImage, const VkImageSubresourceRange& Range, VkFormat InFormat, bool IsCubemap)
		: Image(std::move(InImage))
		, Format(InFormat)
	{
		VkImageViewCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		CreateInfo.image = Image->Image;
		CreateInfo.viewType = IsCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
		CreateInfo.format = Format;
		CreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		CreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		CreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		CreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		CreateInfo.subresourceRange = Range;

		VK_CHECK_RESULT(vkCreateImageView(Image->Device->Device, &CreateInfo, GVulkanAllocator, &View));
	}

	ImageView::~ImageView()
	{
		vkDestroyImageView(Image->Device->Device, View, GVulkanAllocator);
	}

	VkImageView ImageView::GetImageView() const
	{
		return View;
	}

	VkImage ImageView::GetImage() const
	{
		return Image->Image;
	}

	Vec2ui ImageView::GetDimensions() const
	{
		return Image->Dimensions;
	}

	VkFormat ImageView::GetFormat() const
	{
		return Format;
	}
}
