#pragma once

#include <memory>

#include "AssetSystem/AssetHeaders.h"
#include "Core/Core.h"
#include "Math/Math.h"
#include "Vulkan/Image.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes
{
	class ImageAsset;

	enum class ColorSpace
	{
		Linear = 0,
		SRGB,

		Count_
	};

	enum class MipmapGenerationMode
	{
		DoNotGenerate,
		LoadExisting,
		Generate
	};

	class Texture2D : public Asset
	{
	public:
		static std::unique_ptr<Texture2D> Create(String Name, AssetHandle Handle, Vec2ui Dimensions, ImageFormat Format, size_t BytesPerChannel, const void* Data, MipmapGenerationMode MipmapMode);

		const Vulkan::Image& GetRawImage() const;

		const Vulkan::ImageView& GetView(ColorSpace ColorSpace) const;

		Vec2ui GetDimensions() const;

		uint32 GetMipLevelsCount() const;

		VkFormat GetDataFormat() const;

	private:
		Texture2D(String Name, AssetHandle Handle, Vec2ui Dimensions, ImageFormat Format, size_t BytesPerChannel, const void* Data, MipmapGenerationMode MipmapMode);

		Vulkan::ImageView& CreateView(ColorSpace ColorSpace) const;
		
		std::unique_ptr<Vulkan::Image> Image;
		mutable std::unique_ptr<Vulkan::ImageView> Views[static_cast<size_t>(ColorSpace::Count_)];
	};

	class HERMES_API TextureCube
	{
	public:
		static std::unique_ptr<TextureCube> CreateEmpty(Vec2ui InDimensions, VkFormat InFormat, VkImageUsageFlags InUsage, uint32 InMipLevelCount);

		static std::unique_ptr<TextureCube> CreateFromEquirectangularTexture(const Texture2D& EquirectangularTexture, VkFormat PreferredFormat, bool EnableMipMaps = true);

		const Vulkan::Image& GetRawImage() const;

		const Vulkan::ImageView& GetView() const;

		Vec2ui GetDimensions() const;

		uint32 GetMipLevelsCount() const;

		VkFormat GetDataFormat() const;

	private:
		TextureCube(Vec2ui InDimensions, VkFormat InFormat, VkImageUsageFlags InUsage, uint32 InMipLevelCount);

		TextureCube(const Texture2D& EquirectangularTexture, VkFormat PreferredFormat, bool EnableMipMaps);

		std::unique_ptr<Vulkan::Image> Image;
		std::unique_ptr<Vulkan::ImageView> View;
	};
}
