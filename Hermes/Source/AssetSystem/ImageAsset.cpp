﻿#include "ImageAsset.h"

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
		case ImageFormat::RGBA:
		case ImageFormat::RGBX:
			return 4;
		default:
			HERMES_ASSERT(false);
		}
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
		if (Format != ImageFormat::HDR)
		{
			Data.resize(GetMemorySize(), 0x00);
			std::copy_n(InData, GetMemorySize(), Data.data());
		}
		else
		{
			// NOTE: because HDR assets are stored as R32G32B32 on the drive, but quite few GPUs support this format, so we need to unpack it into R32G32B32A32
			UnpackHDRAsset(reinterpret_cast<const float*>(InData));
		}

		Texture = Texture2DResource::CreateFromAsset(*this);
	}

	void ImageAsset::UnpackHDRAsset(const float* RawData)
	{
		Data.resize(static_cast<size_t>(Dimensions.X) * Dimensions.Y * sizeof(float) * 4);
		auto* Dest = reinterpret_cast<float*>(Data.data());
		for (uint32 Y = 0; Y < Dimensions.Y; Y++)
		{
			for (uint32 X = 0; X < Dimensions.X; X++)
			{
				*Dest++ = *RawData++;
				*Dest++ = *RawData++;
				*Dest++ = *RawData++;

				*Dest++ = 1.0f;
			}
		}
	}

	bool ImageAsset::IsValid() const
	{
		return (Data.size() == GetMemorySize());
	}

	size_t ImageAsset::GetMemorySize() const
	{
		return CalculateTotalPixelCount(Dimensions.X, Dimensions.Y, MipLevelCount) * GetBytesPerPixel();
	}

	const Resource* ImageAsset::GetResource() const
	{
		HERMES_ASSERT(Texture);
		return Texture.get();
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
