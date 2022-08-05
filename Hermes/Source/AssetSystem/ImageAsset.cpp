#include "ImageAsset.h"

#include <algorithm>

namespace Hermes
{
	DEFINE_ASSET_TYPE(Image)

	size_t NumberOfChannelInImageFormat(ImageFormat Format)
	{
		switch (Format)
		{
		case ImageFormat::R:
			return 1;
		case ImageFormat::RG:
		case ImageFormat::RA:
			return 2;
		case ImageFormat::HDR:
			return 3;
		case ImageFormat::RGBA:
		case ImageFormat::RGBX:
			return 4;
		default:
			HERMES_ASSERT(false);
		}
		return 0;
	}

	ImageAsset::ImageAsset(const String& Name, Vec2ui InDimensions, ImageFormat InFormat, size_t InBytesPerChannel)
		: Asset(Name, AssetType::Image)
		, Dimensions(InDimensions)
		, Format(InFormat)
		, BytesPerChannel(InBytesPerChannel)
	{
		Data.resize(GetMemorySize(), 0x00);
	}

	ImageAsset::ImageAsset(const String& Name, Vec2ui InDimensions, ImageFormat InFormat, size_t InBytesPerChannel,
	                       const uint8* InData)
		: Asset(Name, AssetType::Image)
		, Dimensions(InDimensions)
		, Format(InFormat)
		, BytesPerChannel(InBytesPerChannel)
	{
		Data.resize(GetMemorySize(), 0x00);
		std::copy_n(InData, GetMemorySize(), Data.data());
	}

	bool ImageAsset::IsValid() const
	{
		return (Data.size() == GetMemorySize());
	}

	size_t ImageAsset::GetMemorySize() const
	{
		return static_cast<size_t>(Dimensions.X) * Dimensions.Y * GetBytesPerPixel();
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

	size_t ImageAsset::GetBytesPerChannel() const
	{
		return BytesPerChannel;
	}

	size_t ImageAsset::GetBytesPerPixel() const
	{
		return BytesPerChannel * NumberOfChannelInImageFormat(Format);
	}
}
