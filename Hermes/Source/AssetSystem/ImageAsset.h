#pragma once

#include <memory>
#include <vector>

#include "Core/Core.h"
#include "AssetSystem/Asset.h"
#include "Math/Vector2.h"

namespace Hermes
{
	class AssetLoader;

	enum class ImageFormat : uint8
	{
		Undefined = 0x00,
		R = 0x01,
		RA = 0x09,
		RG = 0x03,
		RGBX = 0x07,
		RGBA = 0x0F,
		HDR = 0x10
	};

	PACKED_STRUCT_BEGIN
	struct ImageAssetHeader
	{
		uint16 Width;
		uint16 Height;
		ImageFormat Format;
		uint8 BytesPerChannel;
	};
	PACKED_STRUCT_END

	size_t NumberOfChannelInImageFormat(ImageFormat Format);

	class HERMES_API ImageAsset final : public Asset
	{
	public:

		virtual bool IsValid() const override;

		virtual size_t GetMemorySize() const override;

		const uint8* GetRawData() const;
		uint8* GetRawData();

		Vec2ui GetDimensions() const;

		ImageFormat GetImageFormat() const;

		size_t GetBytesPerChannel() const;
		size_t GetBytesPerPixel() const;

	private:
		/*
		 * Constructor that initializes raw data to 0
		 */
		ImageAsset(const String& Name, Vec2ui InDimensions, ImageFormat InFormat, size_t InBytesPerChannel);

		/*
		 * Constructor that copies raw image data from external array
		 * May apply some conversions to make sure that it stores data in optimal manner(e.g. each pixel
		 * could be stored in default types like uint8, 16, 32, 64 etc.)
		 */
		ImageAsset(const String& Name, Vec2ui InDimensions, ImageFormat InFormat, size_t InBytesPerChannel,
		           const uint8* InData);

		friend class AssetLoader;

		std::vector<uint8> Data;
		Vec2ui Dimensions;
		ImageFormat Format;
		size_t BytesPerChannel;
	};
}
