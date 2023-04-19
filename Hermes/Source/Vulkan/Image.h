#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Math/Vector2.h"
#include "Vulkan/Device.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes::Vulkan
{
	enum class CubemapSide
	{
		PositiveX,
		NegativeX,
		PositiveY,
		NegativeY,
		PositiveZ,
		NegativeZ,

		All
	};

	/*
	 * Returns a Vulkan array index for a given cubemap side
	 */
	HERMES_API uint32 CubemapSideToArrayLayer(CubemapSide Side);

	/*
	 * Returns a bitmask with all aspects that are available for a given format set
	 */
	HERMES_API VkImageAspectFlags FullImageAspectMask(VkFormat Format);

	/*
	 * A wrapper around VkImage object and the allocated memory that it is bound to
	 */
	class HERMES_API Image
	{
		MAKE_NON_COPYABLE(Image)
		MAKE_NON_MOVABLE(Image)
		ADD_DEFAULT_DESTRUCTOR(Image)

	public:
		/*
		 * Non-owning constructor
		 */
		Image(std::shared_ptr<Device::VkDeviceHolder> InDevice, VkImage InImage, VkFormat InFormat,
		      Vec2ui InDimensions, bool InIsCubemapCompatible);

		/*
		 * Owning constructor
		 */
		Image(std::shared_ptr<Device::VkDeviceHolder> InDevice, Vec2ui InDimensions, VkImageUsageFlags InUsage,
		      VkFormat InFormat, uint32 InMipLevels, bool InIsCubemapCompatible);

		/*
		 * Creates image view for given subresource range
		 */
		std::unique_ptr<ImageView> CreateImageView(const VkImageSubresourceRange& Range) const;

		/*
		 * Creates image view for given subresource range and with a particular format (that might be different
		 * from the format of the image itself)
		 */
		std::unique_ptr<ImageView> CreateImageView(const VkImageSubresourceRange& Range, VkFormat ViewFormat) const;

		/*
		 * Creates cubemap image view for given subresource range
		 */
		std::unique_ptr<ImageView> CreateCubemapImageView(const VkImageSubresourceRange& Range) const;

		/*
		 * Creates cubemap image view for given subresource range and a particular format (that might be different
		 * from the format of the image itself)
		 */
		std::unique_ptr<ImageView> CreateCubemapImageView(const VkImageSubresourceRange& Range, VkFormat ViewFormat) const;

		/*
		 * Creates a 'default' image view
		 *
		 * Default image view is an image view that represents the whole image, e.g. from (0,0) to (width,height) and
		 * is a cubemap view if the image was created as a cubemap
		 */
		std::unique_ptr<ImageView> CreateDefaultImageView() const;

		/*
		 * Returns a subresource range that covers the whole image, e.g. all mip levels and array layers
		 */
		VkImageSubresourceRange GetFullSubresourceRange() const;

		Vec2ui GetDimensions() const;

		VkFormat GetDataFormat() const;

		VkImageAspectFlags GetFullAspectMask() const;

		uint32 GetMipLevelsCount() const;

		bool IsCubemap() const;

		VkImage GetImage() const;

	protected:
		struct VkImageHolder
		{
			MAKE_NON_COPYABLE(VkImageHolder)
			MAKE_NON_MOVABLE(VkImageHolder)
			ADD_DEFAULT_CONSTRUCTOR(VkImageHolder)

			~VkImageHolder();

			std::shared_ptr<Device::VkDeviceHolder> Device;

			VkImage Image = VK_NULL_HANDLE;
			VmaAllocation Allocation = VK_NULL_HANDLE;
			VkFormat Format = VK_FORMAT_UNDEFINED;
			Vec2ui Dimensions;
			bool IsOwned = false;
		};

		std::shared_ptr<VkImageHolder> Holder;

		uint32 MipLevelCount = 0;
		bool IsCubemapCompatible = false;

		friend class ImageView;
	};

	/*
	 * A wrapper around VkImageView object
	 *
	 * Shares ownership of an VkImage object that it was created from to ensure correct order of destruction
	 */
	class HERMES_API ImageView
	{
		MAKE_NON_COPYABLE(ImageView)
		MAKE_NON_MOVABLE(ImageView)

	public:
		ImageView(std::shared_ptr<Image::VkImageHolder> InImage, const VkImageSubresourceRange& Range, VkFormat InFormat, bool IsCubemap);

		~ImageView();

		VkImageView GetImageView() const;

		VkImage GetImage() const;
		
		Vec2ui GetDimensions() const;

		VkFormat GetFormat() const;

	private:
		std::shared_ptr<Image::VkImageHolder> Image;

		VkImageView View = VK_NULL_HANDLE;

		VkFormat Format;
	};
}
