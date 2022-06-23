#include "VulkanImage.h"

#include "RenderInterface/GenericRenderInterface/CommonTypes.h"
#include "RenderInterface/Vulkan/VulkanDevice.h"
#include "RenderInterface/Vulkan/VulkanCommonTypes.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanImage::VulkanImage(std::shared_ptr<const VulkanDevice> InDevice, VkImage InImage, VkFormat InFormat,
		                         Vec2ui InSize, RenderInterface::ImageUsageType InUsage, bool InIsCubemapCompatible)
			: Holder(std::make_shared<VkImageHolder>(std::move(InDevice), InImage))
			, Size(InSize)
			, DefaultView(VK_NULL_HANDLE)
			, MipLevelCount(1)
			, IsCubemapCompatible(InIsCubemapCompatible)
			, Usage(InUsage)
		{
			Holder->Format = InFormat;
		}

		VulkanImage::VulkanImage(std::shared_ptr<const VulkanDevice> InDevice, Vec2ui InSize,
			RenderInterface::ImageUsageType InUsage, RenderInterface::DataFormat InFormat,
			uint32 InMipLevels, RenderInterface::ImageLayout InitialLayout,
			bool InIsCubemapCompatible)
			: Holder(std::make_shared<VkImageHolder>(std::move(InDevice), VK_NULL_HANDLE))
			, Size(InSize)
			, DefaultView(VK_NULL_HANDLE)
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
			CreateInfo.format = DataFormatToVkFormat(InFormat);
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
			VK_CHECK_RESULT(vmaCreateImage(Holder->Device->GetAllocator(), &CreateInfo, &AllocationInfo, &Holder->Image, &Holder->Allocation, nullptr))

			Holder->IsOwned = true;
			Holder->Format = DataFormatToVkFormat(InFormat);
		}

		VulkanImage::~VulkanImage()
		{
			vkDestroyImageView(Holder->Device->GetDevice(), DefaultView, GVulkanAllocator);
		}

		VulkanImage::VulkanImage(VulkanImage&& Other)
		{
			*this = std::move(Other);
		}

		VulkanImage& VulkanImage::operator=(VulkanImage&& Other)
		{
			std::swap(Holder, Other.Holder);
			std::swap(Size, Other.Size);
			std::swap(DefaultView, Other.DefaultView);
			std::swap(MipLevelCount, Other.MipLevelCount);
			std::swap(IsCubemapCompatible, Other.IsCubemapCompatible);
			std::swap(Usage, Other.Usage);
			
			return *this;
		}

		std::unique_ptr<RenderInterface::ImageView> VulkanImage::CreateImageView(
			const RenderInterface::ImageViewDescription& Description) const
		{
			return std::make_unique<VulkanImageView>(Holder, Description, IsCubemapCompatible);
		}

		std::unique_ptr<RenderInterface::ImageView> VulkanImage::CreateDefaultImageView() const
		{
			RenderInterface::ImageViewDescription Description = {};
			Description.Aspects = ImageAspectFromDataFormat(VkFormatToDataFormat(Holder->Format));
			Description.BaseMipLevel = 0;
			Description.MipLevelCount = MipLevelCount;

			return std::make_unique<VulkanImageView>(Holder, Description, IsCubemapCompatible);
		}

		Vec2ui VulkanImage::GetSize() const
		{
			return Size;
		}

		RenderInterface::DataFormat VulkanImage::GetDataFormat() const
		{
			return VkFormatToDataFormat(Holder->Format);
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
			return Holder->Image;
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

		VulkanImage::VkImageHolder::VkImageHolder(std::shared_ptr<const VulkanDevice> InDevice, VkImage InImage)
			: Device(std::move(InDevice))
			, Image(InImage)
		{
		}

		VulkanImage::VkImageHolder::~VkImageHolder()
		{
			if (IsOwned)
			{
				vmaDestroyImage(Device->GetAllocator(), Image, Allocation);
			}
		}

		VulkanImage::VkImageHolder::VkImageHolder(VkImageHolder&& Other)
		{
			*this = std::move(Other);
		}

		VulkanImage::VkImageHolder& VulkanImage::VkImageHolder::operator=(VkImageHolder&& Other)
		{
			if (this != &Other)
			{
				std::swap(Device, Other.Device);
				std::swap(Image, Other.Image);
				std::swap(Allocation, Other.Allocation);
				std::swap(IsOwned, Other.IsOwned);
			}
			return *this;
		}

		void VulkanImage::CreateDefaultView() const
		{
			VkImageViewCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			CreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			CreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			CreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			CreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			CreateInfo.format = Holder->Format;
			CreateInfo.image = Holder->Image;
			CreateInfo.subresourceRange.aspectMask = VkAspectFlagsFromVkFormat(Holder->Format);
			CreateInfo.subresourceRange.baseArrayLayer = 0;
			CreateInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
			CreateInfo.subresourceRange.baseMipLevel = 0;
			CreateInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
			CreateInfo.viewType = IsCubemapCompatible ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;

			VK_CHECK_RESULT(vkCreateImageView(Holder->Device->GetDevice(), &CreateInfo, GVulkanAllocator, &DefaultView))
		}

		VulkanImageView::VulkanImageView(std::shared_ptr<VulkanImage::VkImageHolder> InImage,
		                                 const RenderInterface::ImageViewDescription& Description,
		                                 bool IsCubemap)
			: Image(std::move(InImage))
		{
			VkImageViewCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			CreateInfo.image = Image->Image;
			CreateInfo.viewType = IsCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
			CreateInfo.format = Image->Format;
			CreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			CreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			CreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			CreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			CreateInfo.subresourceRange.aspectMask = ImageAspectToVkImageAspectFlags(Description.Aspects);
			CreateInfo.subresourceRange.baseArrayLayer = 0;
			CreateInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
			CreateInfo.subresourceRange.baseMipLevel = Description.BaseMipLevel;
			CreateInfo.subresourceRange.levelCount = Description.MipLevelCount;

			VK_CHECK_RESULT(vkCreateImageView(Image->Device->GetDevice(), &CreateInfo, GVulkanAllocator, &View));
		}

		VulkanImageView::~VulkanImageView()
		{
			vkDestroyImageView(Image->Device->GetDevice(), View, GVulkanAllocator);
		}

		VulkanImageView::VulkanImageView(VulkanImageView&& Other)
		{
			*this = std::move(Other);
		}

		VulkanImageView& VulkanImageView::operator=(VulkanImageView&& Other)
		{
			std::swap(Image, Other.Image);
			std::swap(View, Other.View);

			return *this;
		}

		VkImageView VulkanImageView::GetImageView() const
		{
			return View;
		}
	}
}
