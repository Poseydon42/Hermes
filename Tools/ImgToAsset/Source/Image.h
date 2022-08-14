#pragma once

#include <optional>
#include <vector>

#include "AssetSystem/ImageAsset.h"

/*
 * Wrapper around underlying image data
 * Should be used as a representation of an intermediate image in a series of processing steps
 */
class Image
{
public:
	Image(uint16_t InWidth, uint16_t InHeight, Hermes::ImageFormat InFormat, size_t InBytesPerChannel,
	      std::optional<const void*> InData = {});

	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;
	Image(Image&&) = default;
	Image& operator=(Image&&) = default;
	~Image() = default;

	/*
	 * Creates a copy of source image downscaling it to given dimensions using linear filtering
	 */
	static std::unique_ptr<Image> CreateDownscaled(const Image& Source, uint16_t Width, uint16_t Height);

	uint16_t GetWidth() const;
	uint16_t GetHeight() const;

	Hermes::ImageFormat GetFormat() const;
	size_t GetChannelCount() const;
	size_t GetBytesPerChannel() const;
	size_t GetBytesPerPixel() const;

	const void* GetData() const;
	void* GetData();

	size_t GetDataSize() const;

	/*
	 * Floating point manipulations
	 * All the following functions convert the underlying image data to and from
	 * floating point pixel representation. If any of the R, G, B and A channels are not
	 * present in the image, the value of their floating point representation after read
	 * operation is undefined and they are ignored during write operations
	 */
	struct Pixel
	{
		float R;
		float G;
		float B;
		float A;

		Pixel operator+(const Pixel& Right) const;
		Pixel operator*(float Value) const;
	};

	/*
	 * Returns value of texel with given coordinates
	 */
	Pixel Sample(uint16_t X, uint16_t Y) const;

	/*
	 * Returns result of linear filtering of four texels closest to the given normalized coordinates
	 */
	Pixel Sample(float X, float Y) const;

	void Store(uint16_t X, uint16_t Y, const Pixel& Value);

private:
	uint16_t Width = 0;
	uint16_t Height = 0;
	size_t BytesPerChannel = 0;

	Hermes::ImageFormat Format = Hermes::ImageFormat::Undefined;

	std::vector<uint8_t> Data;

	uint8_t* GetPointerToFirstByteOfPixel(uint16_t X, uint16_t Y);
	const uint8_t* GetPointerToFirstByteOfPixel(uint16_t X, uint16_t Y) const;
};
