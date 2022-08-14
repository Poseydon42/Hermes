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

	size_t CalculateTotalPixelCount(size_t Width, size_t Height, size_t MipLevelCount)
	{
		size_t Result = 0;
		while (MipLevelCount--)
		{
			Result += Width * Height;

			Width = Math::Max<size_t>(Width / 2, 1);
			Height = Math::Max<size_t>(Height / 2, 1);
		}
		return Result;
	}

	ImageAsset::ImageAsset(const String& Name, Vec2ui InDimensions, ImageFormat InFormat, size_t InBytesPerChannel,
	                       uint8 InMipLevelCount)
		: Asset(Name, AssetType::Image)
		, Dimensions(InDimensions)
		, Format(InFormat)
		, BytesPerChannel(InBytesPerChannel)
		, MipLevelCount(InMipLevelCount)
	{
		Data.resize(GetMemorySize(), 0x00);
	}

	ImageAsset::ImageAsset(const String& Name, Vec2ui InDimensions, ImageFormat InFormat, size_t InBytesPerChannel,
	                       uint8 InMipLevelCount, const uint8* InData)
		: Asset(Name, AssetType::Image)
		, Dimensions(InDimensions)
		, Format(InFormat)
		, BytesPerChannel(InBytesPerChannel)
		, MipLevelCount(InMipLevelCount)
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
		return CalculateTotalPixelCount(Dimensions.X, Dimensions.Y, MipLevelCount) * GetBytesPerPixel();
	}

	const uint8* ImageAsset::GetRawData(uint8 MipLevel) const
	{
		return const_cast<ImageAsset*>(this)->GetRawData(MipLevel);
	}

	uint8* ImageAsset::GetRawData(uint8 MipLevel)
	{
		HERMES_ASSERT(!Data.empty());
		HERMES_ASSERT(MipLevel < MipLevelCount);
		
		// NOTE : size of all previous mip levels is the offset to the current one
		size_t Offset = CalculateTotalPixelCount(Dimensions.X, Dimensions.Y, MipLevel) * GetBytesPerPixel();

		return Data.data() + Offset;
	}

	Vec2ui ImageAsset::GetDimensions(uint8 MipLevel) const
	{
		HERMES_ASSERT(MipLevel < MipLevelCount);
		Vec2ui Result = Dimensions;
		while (MipLevel--)
		{
			Result.X = Math::Max<uint32>(Result.X / 2, 1);
			Result.Y = Math::Max<uint32>(Result.Y / 2, 1);
		}
		return Result;
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

	bool ImageAsset::HasPrecomputedMips() const
	{
		return (MipLevelCount > 1);
	}

	uint8 ImageAsset::GetMipLevelCount() const
	{
		return MipLevelCount;
	}
}
