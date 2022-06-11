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
			/*
			 * Non-owning constructor
			 */
			VulkanImage(std::shared_ptr<const VulkanDevice> InDevice, VkImage InImage, VkFormat InFormat, Vec2ui InSize,
			            RenderInterface::ImageUsageType InUsage, bool InIsCubemapCompatible = false);

			/*
			 * Owning constructor
			 */
			VulkanImage(std::shared_ptr<const VulkanDevice> InDevice, Vec2ui InSize,
			            RenderInterface::ImageUsageType InUsage, RenderInterface::DataFormat InFormat,
			            uint32 InMipLevels, RenderInterface::ImageLayout InitialLayout, bool InIsCubemapCompatible = false);

			virtual ~VulkanImage() override;
			VulkanImage(VulkanImage&& Other);
			VulkanImage& operator=(VulkanImage&& Other);

			virtual Vec2ui GetSize() const override;

			virtual RenderInterface::DataFormat GetDataFormat() const override;

			virtual uint32 GetMipLevelsCount() const override;

			virtual RenderInterface::ImageUsageType GetUsageFlags() const override;

			VkImage GetImage() const;

			/**
			 * \brief Returns 'default'(e.g. all mips, all layers, initial format) image view of this image
			 * \return VkImageView that covers whole image with its initial format
			 */
			VkImageView GetDefaultView() const;

			bool GetIsCubemapCompatible() const;

		protected:
			std::shared_ptr<const VulkanDevice> Device;
			VkImage Handle;
			Vec2ui Size;
			VkFormat Format;
			mutable VkImageView DefaultView;
			VmaAllocation Allocation;
			bool IsOwned;
			uint32 MipLevelCount;
			bool IsCubemapCompatible;
			RenderInterface::ImageUsageType Usage;

			virtual void CreateDefaultView() const;
		};

		class HERMES_API VulkanCubemapImage : public VulkanImage
		{
			MAKE_NON_COPYABLE(VulkanCubemapImage);

		public:
			VulkanCubemapImage(std::shared_ptr<const VulkanDevice> InDevice, Vec2ui InSize,
			            RenderInterface::ImageUsageType InUsage, RenderInterface::DataFormat InFormat,
			            uint32 InMipLevels, RenderInterface::ImageLayout InitialLayout);

			virtual ~VulkanCubemapImage() override = default;

		private:
			virtual void CreateDefaultView() const override;
		};
	}
}
