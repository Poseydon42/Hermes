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
			VulkanImage(std::shared_ptr<VulkanDevice> InDevice, VkImage InImage, VkFormat InFormat, Vec2ui InSize);

			/*
			 * Owning constructor
			 */
			VulkanImage(std::shared_ptr<VulkanDevice> InDevice, Vec2ui InSize, RenderInterface::ImageUsageType Usage, RenderInterface::DataFormat InFormat, uint32 InMipLevels, RenderInterface::ImageLayout InitialLayout);

			~VulkanImage() override;
			VulkanImage(VulkanImage&& Other);
			VulkanImage& operator=(VulkanImage&& Other);

			Vec2ui GetSize() const override;
			
			RenderInterface::DataFormat GetDataFormat() const override;

			uint32 GetMipLevelsCount() const override;

			VkImage GetImage() const;

			/**
			 * \brief Returns 'default'(e.g. all mips, all layers, initial format) image view of this image
			 * \return VkImageView that covers whole image with its initial format
			 */
			VkImageView GetDefaultView() const;

		private:
			std::shared_ptr<VulkanDevice> Device;
			VkImage Handle;
			Vec2ui Size;
			VkFormat Format;
			VkImageView DefaultView;
			VmaAllocation Allocation;
			bool IsOwned;
			uint32 MipLevelCount;

			void CreateDefaultView();
		};
	}
}
