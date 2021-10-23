#pragma once

#include <memory>

#include "Core/Core.h"
#include "AssetSystem/Asset.h"
#include "Math/Vector2.h"

namespace Hermes
{
	class AssetLoader;

	// TODO : maybe add more?
	enum class ImageFormat : uint8
	{
		R8 = 0x01,
		R16 = 0x02,
		R32 = 0x03,
		R8G8 = 0x05,
		R16G16 = 0x0A,
		B8G8R8X8 = 0x54,
		B8G8R8A8 = 0x55
	};

	uint8 BytesPerPixelForImageFormat(ImageFormat Format);

	class HERMES_API ImageAsset final : public Asset
	{
	public:

		virtual bool IsValid() const override;

		virtual size_t GetMemorySize() const override;

		const uint8* GetRawData() const;
		uint8* GetRawData();

		Vec2ui GetDimensions() const;

		ImageFormat GetImageFormat() const;

		uint8 GetBitsPerPixel() const;

	private:
		/*
		 * Constructor that initializes raw data to 0
		 */
		ImageAsset(const String& Name, Vec2ui InDimensions, ImageFormat InFormat);

		/*
		 * Constructor that copies raw image data from external array
		 * May apply some conversions to make sure that it stores data in optimal manner(e.g. each pixel
		 * could be stored in default types like uint8, 16, 32, 64 etc.)
		 */
		ImageAsset(const String& Name, Vec2ui InDimensions, ImageFormat InFormat, const uint8* InData);

		friend class AssetLoader;

		std::vector<uint8> Data;
		Vec2ui Dimensions;
		ImageFormat Format;
	};
}
