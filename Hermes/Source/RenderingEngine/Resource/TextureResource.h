#pragma once

#include <memory>

#include "Core/Core.h"
#include "Math/Math.h"
#include "RenderingEngine/Resource/Resource.h"
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

	class Texture2DResource : public Resource
	{
	public:
		static std::unique_ptr<Texture2DResource> CreateFromAsset(const ImageAsset& Source, bool EnableMipMaps = true);

		const Vulkan::Image& GetRawImage() const;

		const Vulkan::ImageView& GetView(ColorSpace ColorSpace) const;

		Vec2ui GetDimensions() const;

		uint32 GetMipLevelsCount() const;

		VkFormat GetDataFormat() const;

	private:
		explicit Texture2DResource(const ImageAsset& Source, bool EnableMipMaps = true);

		Vulkan::ImageView& CreateView(ColorSpace ColorSpace) const;
		
		std::unique_ptr<Vulkan::Image> Image;
		mutable std::unique_ptr<Vulkan::ImageView> Views[static_cast<size_t>(ColorSpace::Count_)];
	};

	class HERMES_API TextureCubeResource : public Resource
	{
	public:
		static std::unique_ptr<TextureCubeResource> CreateEmpty(String Name, Vec2ui InDimensions, VkFormat InFormat, VkImageUsageFlags InUsage, uint32 InMipLevelCount);

		static std::unique_ptr<TextureCubeResource> CreateFromEquirectangularTexture(String Name, const Texture2DResource& EquirectangularTexture, VkFormat PreferredFormat, bool EnableMipMaps = true);

		const Vulkan::Image& GetRawImage() const;

		const Vulkan::ImageView& GetView() const;

		Vec2ui GetDimensions() const;

		uint32 GetMipLevelsCount() const;

		VkFormat GetDataFormat() const;

	private:
		TextureCubeResource(String Name, Vec2ui InDimensions, VkFormat InFormat, VkImageUsageFlags InUsage, uint32 InMipLevelCount);

		TextureCubeResource(String Name, const Texture2DResource& EquirectangularTexture, VkFormat PreferredFormat, bool EnableMipMaps);

		std::unique_ptr<Vulkan::Image> Image;
		std::unique_ptr<Vulkan::ImageView> View;
	};
}
