#include "Image.h"

static size_t BytesPerPixelForFormat(ImageFormat Format)
{
	switch (Format)
	{
	default:
	case ImageFormat::Undefined:
		return 0;
	case ImageFormat::R8:
		return 1;
	case ImageFormat::R16:
	case ImageFormat::R8G8:
		return 2;
	case ImageFormat::R32:
	case ImageFormat::R16G16:
	case ImageFormat::R8G8B8X8:
	case ImageFormat::R8G8B8A8:
		return 4;
	case ImageFormat::R16G16B16X16:
	case ImageFormat::R16G16B16A16:
		return 8;
	case ImageFormat::HDR96:
		return 12;
	}
}

Image::Image(uint16_t InWidth, uint16_t InHeight, ImageFormat InFormat, std::optional<void*> InData)
	: Width(InWidth)
	, Height(InHeight)
	, Format(InFormat)
{
	size_t TotalNumberOfBytes = static_cast<size_t>(Width) * Height * BytesPerPixelForFormat(Format);
	Data.resize(TotalNumberOfBytes);
	if (InData.has_value())
	{
		memcpy(GetData(), InData.value(), TotalNumberOfBytes);
	}
}

uint16_t Image::GetWidth() const
{
	return Width;
}

uint16_t Image::GetHeight() const
{
	return Height;
}

ImageFormat Image::GetFormat() const
{
	return Format;
}

size_t Image::GetBytesPerPixel() const
{
	return BytesPerPixelForFormat(Format);
}

const void* Image::GetData() const
{
	return Data.data();
}

void* Image::GetData()
{
	return Data.data();
}

size_t Image::GetDataSize() const
{
	return Data.size();
}
