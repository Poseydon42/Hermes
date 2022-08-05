#include "Image.h"

static size_t ChannelsPerFormat(ImageFormat Format)
{
	switch (Format)
	{
	default:
	case ImageFormat::Undefined:
		return 0;
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
	}
}

Image::Image(uint16_t InWidth, uint16_t InHeight, ImageFormat InFormat, size_t InBytesPerChannel,
             std::optional<void*> InData)
	: Width(InWidth)
	, Height(InHeight)
	, BytesPerChannel(InBytesPerChannel)
	, Format(InFormat)
{
	size_t TotalNumberOfBytes = static_cast<size_t>(Width) * Height * BytesPerChannel * ChannelsPerFormat(Format);
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

size_t Image::GetChannelCount() const
{
	return ChannelsPerFormat(Format);
}

size_t Image::GetBytesPerChannel() const
{
	return BytesPerChannel;
}

size_t Image::GetBytesPerPixel() const
{
	return BytesPerChannel * ChannelsPerFormat(Format);
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
