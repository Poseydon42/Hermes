#include "ImageAsset.h"

#include <algorithm>

namespace Hermes
{
	uint8 BytesPerPixelForImageFormat(ImageFormat Format)
	{
		switch (Format)
		{
		case ImageFormat::R8:
			return 8;
		case ImageFormat::R16:
		case ImageFormat::R8G8:
			return 16;
		case ImageFormat::B8G8R8X8:
		case ImageFormat::B8G8R8A8:
		case ImageFormat::R16G16:
		case ImageFormat::R32:
			return 32;
		default:
			HERMES_ASSERT(false);
		}
		return 0;
	}

	ImageAsset::ImageAsset(const String& Name, Vec2ui InDimensions, ImageFormat InFormat)
		: Asset(Name, AssetType::Image)
		, Dimensions(InDimensions)
		, Format(InFormat)
	{
		Data.resize(GetMemorySize(), 0x00);
	}

	ImageAsset::ImageAsset(const String& Name, Vec2ui InDimensions, ImageFormat InFormat, const uint8* InData)
		: Asset(Name, AssetType::Image)
		, Dimensions(InDimensions)
		, Format(InFormat)
	{
		Data.resize(GetMemorySize(), 0x00);
		std::copy_n(InData, GetMemorySize(), Data.data());
	}

	bool ImageAsset::IsValid() const
	{
		return (Data.size() == Dimensions.X * Dimensions.Y * GetBitsPerPixel() / 8);
	}

	size_t ImageAsset::GetMemorySize() const
	{
		return static_cast<size_t>(Dimensions.X) * Dimensions.Y * GetBitsPerPixel() / 8;
	}

	const uint8* ImageAsset::GetRawData() const
	{
		HERMES_ASSERT(!Data.empty());
		return Data.data();
	}

	uint8* ImageAsset::GetRawData()
	{
		HERMES_ASSERT(!Data.empty());
		return Data.data();
	}

	Vec2ui ImageAsset::GetDimensions() const
	{
		return Dimensions;
	}

	ImageFormat ImageAsset::GetImageFormat() const
	{
		return Format;
	}

	uint8 ImageAsset::GetBitsPerPixel() const
	{
		return BytesPerPixelForImageFormat(Format);
	}
}
