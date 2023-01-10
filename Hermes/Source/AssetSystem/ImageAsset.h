#pragma once

#include <vector>

#include "Core/Core.h"
#include "AssetSystem/Asset.h"
#include "Math/Vector2.h"

namespace Hermes
{
	/*
	 * NOTE : overall structure of an image asset file:
	 *
	 * 1) AssetHeader
	 * 2) ImageAssetHeader
	 * 3) Binary image data
	 *
	 * Binary image data consists of one or more chunks that contain pixel data for each mip level,
	 * starting with mip 0. Each chunk must have a size of <width of current mip level> * <height of
	 * current mip level> * <bytes per pixel for image format>. Dimensions of mip level N are calculated
	 * as following:
	 * Dimension(N) = Max(Truncate(Dimension(N - 1) / 2, 1)
	 * for both width and height
	 */

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

	size_t NumberOfChannelInImageFormat(ImageFormat Format);

	size_t CalculateTotalPixelCount(size_t Width, size_t Height, size_t MipLevelCount);

	class HERMES_API ImageAsset final : public Asset
	{
	public:

		virtual bool IsValid() const override;

		virtual size_t GetMemorySize() const override;

		const uint8* GetRawData(uint8 MipLevel = 0) const;
		uint8* GetRawData(uint8 MipLevel = 0);

		Vec2ui GetDimensions(uint8 MipLevel = 0) const;

		ImageFormat GetImageFormat() const;

		size_t GetBytesPerChannel() const;
		size_t GetBytesPerPixel() const;

		bool HasPrecomputedMips() const;
		uint8 GetMipLevelCount() const;

	private:
		/*
		 * Constructor that initializes raw data to 0
		 */
		ImageAsset(const String& Name, Vec2ui InDimensions, ImageFormat InFormat, size_t InBytesPerChannel,
		           uint8 InMipLevelCount);

		/*
		 * Constructor that copies raw image data from external array
		 * May apply some conversions to make sure that it stores data in optimal manner(e.g. each pixel
		 * could be stored in default types like uint8, 16, 32, 64 etc.)
		 */
		ImageAsset(const String& Name, Vec2ui InDimensions, ImageFormat InFormat, size_t InBytesPerChannel,
		           uint8 InMipLevelCount, const uint8* InData);

		friend class AssetLoader;

		std::vector<uint8> Data;
		Vec2ui Dimensions;
		ImageFormat Format;
		size_t BytesPerChannel;
		uint8 MipLevelCount;

		void UnpackHDRAsset(const float* RawData);
	};
}
