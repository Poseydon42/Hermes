﻿#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Math/Math.h"
#include "Vulkan/Image.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes
{
	class ImageAsset;

	class HERMES_API Texture
	{
		MAKE_NON_COPYABLE(Texture)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(Texture)
		ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Texture)

	public:
		static std::shared_ptr<Texture> CreateFromAsset(const ImageAsset& Source, bool UseAsSRGB, bool EnableMipMaps = true);

		const Vulkan::Image& GetRawImage() const;

		const Vulkan::ImageView& GetDefaultView() const;

		Vec2ui GetDimensions() const;

		uint32 GetMipLevelsCount() const;

		VkFormat GetDataFormat() const;

		bool IsReady() const;

	private:
		explicit Texture(const ImageAsset& Source, bool UseAsSRGB, bool EnableMipMaps = true);

	protected:
		Texture() = default;

		bool DataUploadFinished = false;
		std::unique_ptr<Vulkan::Image> Image;
		std::unique_ptr<Vulkan::ImageView> DefaultView;
		Vec2ui Dimensions;
	};

	class HERMES_API CubemapTexture : public Texture
	{
		MAKE_NON_COPYABLE(CubemapTexture)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(CubemapTexture)
		ADD_DEFAULT_VIRTUAL_DESTRUCTOR(CubemapTexture)

	public:
		static std::unique_ptr<CubemapTexture> CreateEmpty(Vec2ui InDimensions, VkFormat InFormat,
		                                                   VkImageUsageFlags InUsage, uint32 InMipLevelCount);

		static std::unique_ptr<CubemapTexture> CreateFromEquirectangularTexture(
			const Texture& EquirectangularTexture, VkFormat PreferredFormat,
			bool EnableMipMaps = true);

	private:
		CubemapTexture(Vec2ui InDimensions, VkFormat InFormat,
		               VkImageUsageFlags InUsage, uint32 InMipLevelCount);

		CubemapTexture(const Texture& EquirectangularTexture, VkFormat PreferredFormat, bool EnableMipMaps);
	};
}
