#pragma once

#include <optional>
#include <vector>

// NOTE : keep in sync with engine code
enum class ImageFormat : uint8_t
{
	Undefined = 0x00,
	R8 = 0x01,
	R16 = 0x02,
	R32 = 0x03,
	R8G8 = 0x05,
	R16G16 = 0x0A,
	R8G8B8X8 = 0x54,
	R8G8B8A8 = 0x55,
	R16G16B16X16 = 0xA8,
	R16G16B16A16 = 0xAA,
	HDR96 = 0xFC
};

/*
 * Wrapper around underlying image data
 * Should be used as a representation of an intermediate image in a series of processing steps
 */
class Image
{
public:
	Image(uint16_t InWidth, uint16_t InHeight, ImageFormat InFormat, std::optional<void*> InData = {});

	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;
	Image(Image&&) = default;
	Image& operator=(Image&&) = default;
	~Image() = default;

	/*
	 * Creates a copy of source image with the same dimensions and format and applies callback
	 * to every pixel of source image to compute pixel of the result image
	 * Callback should have following signature:
	 * void Callback(uint8_t* Dest, const uint8_t* Source, ImageFormat Format);
	 * Callback should write exactly bytes per pixel of the source image bytes on each call
	 */
	template<typename FuncType>
	Image CopyAndApplyCallbackPerPixel(const Image& Source, FuncType Callback);

	uint16_t GetWidth() const;
	uint16_t GetHeight() const;

	ImageFormat GetFormat() const;
	size_t GetBytesPerPixel() const;

	const void* GetData() const;
	void* GetData();

	size_t GetDataSize() const;

private:
	uint16_t Width = 0;
	uint16_t Height = 0;

	ImageFormat Format = ImageFormat::Undefined;

	std::vector<uint8_t> Data;
};

template<typename FuncType>
Image Image::CopyAndApplyCallbackPerPixel(const Image& Source, FuncType Callback)
{
	Image Result(Source.GetWidth(), Source.GetHeight(), Source.GetFormat());

	size_t BytesPerPixel = Source.GetBytesPerPixel();
	size_t PixelCount = Source.GetWidth() * Source.GetHeight();

	const auto* SourcePixel = static_cast<const uint8_t*>(Source.GetData());
	uint8_t* DestPixel = static_cast<uint8_t*>(Result.GetData());
	for (size_t PixelIndex = 0; PixelIndex < PixelCount; PixelIndex++)
	{
		Callback(DestPixel, SourcePixel, Source.GetFormat());
		SourcePixel += BytesPerPixel;
		DestPixel += BytesPerPixel;
	}

	return Result;
}
