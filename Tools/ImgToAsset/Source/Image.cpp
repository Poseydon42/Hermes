#include "Image.h"

#include <concepts>

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

static bool HasRedChannel(ImageFormat Format)
{
	return Format != ImageFormat::Undefined;
}

static bool HasGreenChannel(ImageFormat Format)
{
	static constexpr ImageFormat FormatsWithGreenChannel[] = {
		ImageFormat::RG, ImageFormat::RGBA, ImageFormat::RGBX, ImageFormat::HDR
	};
	return std::ranges::find(FormatsWithGreenChannel, Format) != std::end(FormatsWithGreenChannel);
}

static bool HasBlueChannel(ImageFormat Format)
{
	static constexpr ImageFormat FormatsWithBlueChannel[] = {
		ImageFormat::RGBA, ImageFormat::RGBX, ImageFormat::HDR
	};
	return std::ranges::find(FormatsWithBlueChannel, Format) != std::end(FormatsWithBlueChannel);
}

static bool HasAlphaChannel(ImageFormat Format)
{
	static constexpr ImageFormat FormatsWithAlphaChannel[] = {
		ImageFormat::RA, ImageFormat::RGBA
	};
	return std::ranges::find(FormatsWithAlphaChannel, Format) != std::end(FormatsWithAlphaChannel);
}

static bool HasPaddingAlphaChannel(ImageFormat Format)
{
	return Format == ImageFormat::RGBX;
}

template<typename T>
T Clamp(T Min, T Value, T Max)
{
	return std::max(Min, std::min(Value, Max));
}

/*
 * Reads N bytes from the specified location and returns integer of type specified by template value
 * that is creates using at most sizeof(ReturnType) first bytes, filling unread bytes with zero.
 * NOTE : composes number as if it was stored in little endian order in memory
 */
template<typename ResultType>
requires std::is_integral_v<ResultType>
static ResultType ReadBytesAndComposeInteger(const uint8_t* Data, size_t NumberOfBytes)
{
	ResultType Result = 0;
	size_t ResultSize = sizeof(Result);

	for (size_t BytesRead = 0; BytesRead < std::min(ResultSize, NumberOfBytes); BytesRead++)
	{
		uint8_t Byte = *Data++;
		Result |= Byte << (BytesRead * 8); // LSB stored first, each following byte is more significant => needs to be shifted up
	}

	return Result;
}

/*
 * Writes N least significant bytes of the input integer to the location specified in little-endian order(LSB first)
 */
static void WriteBytesFromDecomposedInteger(uint8_t* Location, size_t NumberOfBytes, std::integral auto Value)
{
	while (NumberOfBytes--)
	{
		*Location++ = static_cast<uint8_t>(Value & 0xFF);
		Value >>= 8;
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

Image::Pixel Image::Pixel::operator+(const Pixel& Right) const
{
	return { R + Right.R, G + Right.G, B + Right.B, A + Right.A };
}

Image::Pixel Image::Pixel::operator*(float Value) const
{
	return { R * Value, G * Value, B * Value, A * Value };
}

Image::Pixel Image::Sample(uint16_t X, uint16_t Y) const
{
	Pixel Result = {};

	// Sampling mode is 'clamp-to-edge'
	X = std::min(std::max<uint16_t>(X, 0), Width);
	Y = std::min(std::max<uint16_t>(Y, 0), Height);

	const auto* PixelData = GetPointerToFirstByteOfPixel(X, Y);

	if (Format == ImageFormat::HDR)
	{
		// NOTE : special handling for HDR as they are stored as floats instead of integers
		const auto* FloatPixelData = reinterpret_cast<const float*>(PixelData);
		Result.R = *FloatPixelData++;
		Result.G = *FloatPixelData++;
		Result.B = *FloatPixelData++;

		return Result;
	}

	// TODO : handle images with bytes per channel other than 1 or 2
	// NOTE : we use the fact that channels are always laid out in memory in order red, green, blue, alpha
	// NOTE : we read all channels as 2 bytes wide, but then mask out the garbage if the channel is only 1 byte long
	// NOTE : we don't support 4+ bytes per channel yet(and do we really need to?)
	// Max value that can be stored in integer representation of channel data
	auto MaxValueInChannel = static_cast<float>((1 << (BytesPerChannel * 8)) - 1);
	if (HasRedChannel(Format))
	{
		auto RawRed = ReadBytesAndComposeInteger<uint16_t>(PixelData, BytesPerChannel);
		Result.R = static_cast<float>(RawRed) / MaxValueInChannel;

		PixelData += BytesPerChannel;
	}
	if (HasGreenChannel(Format))
	{
		auto RawGreen = ReadBytesAndComposeInteger<uint16_t>(PixelData, BytesPerChannel);
		Result.G = static_cast<float>(RawGreen) / MaxValueInChannel;

		PixelData += BytesPerChannel;
	}
	if (HasBlueChannel(Format))
	{
		auto RawBlue = ReadBytesAndComposeInteger<uint16_t>(PixelData, BytesPerChannel);
		Result.B = static_cast<float>(RawBlue) / MaxValueInChannel;

		PixelData += BytesPerChannel;
	}
	if (HasAlphaChannel(Format) || HasPaddingAlphaChannel(Format))
	{
		auto RawAlpha = ReadBytesAndComposeInteger<uint16_t>(PixelData, BytesPerChannel);
		if (!HasPaddingAlphaChannel(Format)) // NOTE : write the result only if the value is meaningful
			Result.A = static_cast<float>(RawAlpha) / MaxValueInChannel;

		PixelData += BytesPerChannel;
	}

	return Result;
}

Image::Pixel Image::Sample(float X, float Y) const
{
	float ScaledX = X * static_cast<float>(Width);
	float ScaledY = Y * static_cast<float>(Height);
	
	auto X00 = static_cast<uint16_t>(Clamp(0.0f, roundf(ScaledX - 1.0f), static_cast<float>(Width - 1)));
	auto Y00 = static_cast<uint16_t>(Clamp(0.0f, roundf(ScaledY - 1.0f), static_cast<float>(Height - 1)));
	auto X01 = static_cast<uint16_t>(Clamp(0.0f, roundf(ScaledX - 1.0f), static_cast<float>(Width - 1)));
	auto Y01 = static_cast<uint16_t>(Clamp(0.0f, roundf(ScaledY), static_cast<float>(Height - 1)));
	auto X10 = static_cast<uint16_t>(Clamp(0.0f, roundf(ScaledX), static_cast<float>(Width - 1)));
	auto Y10 = static_cast<uint16_t>(Clamp(0.0f, roundf(ScaledY - 1.0f), static_cast<float>(Height - 1)));
	auto X11 = static_cast<uint16_t>(Clamp(0.0f, roundf(ScaledX), static_cast<float>(Width - 1)));
	auto Y11 = static_cast<uint16_t>(Clamp(0.0f, roundf(ScaledY), static_cast<float>(Height - 1)));
	
	auto P00 = Sample(X00, Y00);
	auto P01 = Sample(X01, Y01);
	auto P10 = Sample(X10, Y10);
	auto P11 = Sample(X11, Y11);

	float XCoefficient = std::max(0.0f, ScaledX - 0.5f) - static_cast<float>(X00);
	float YCoefficient = std::max(0.0f, ScaledY - 0.5f) - static_cast<float>(Y00);

	// NOTE : linear interpolation on X and Y
	Pixel Result = 
		P00 * (1.0f - XCoefficient) * (1.0f - YCoefficient) +
		P10 *         XCoefficient  * (1.0f - YCoefficient) +
		P01 * (1.0f - XCoefficient) *         YCoefficient  +
		P11 *         XCoefficient  *         YCoefficient;

	return Result;
}

void Image::Store(uint16_t X, uint16_t Y, const Pixel& Value)
{
	if (X < 0 || X >= Width || Y < 0 || Y >= Height)
		return;

	auto* PixelData = GetPointerToFirstByteOfPixel(X, Y);

	// NOTE : see comments inside Sample() for explanation
	if (Format == ImageFormat::HDR)
	{
		auto* FloatPixelData = reinterpret_cast<float*>(PixelData);
		*FloatPixelData++ = Value.R;
		*FloatPixelData++ = Value.G;
		*FloatPixelData++ = Value.B;
	}
	else
	{
		auto ScalingCoefficient = static_cast<float>((1 << (BytesPerChannel * 8)) - 1);
		if (HasRedChannel(Format))
		{
			auto RawRed = static_cast<uint16_t>(Value.R * ScalingCoefficient);
			WriteBytesFromDecomposedInteger(PixelData, BytesPerChannel, RawRed);

			PixelData += BytesPerChannel;
		}
		if (HasGreenChannel(Format))
		{
			auto RawGreen = static_cast<uint16_t>(Value.G * ScalingCoefficient);
			WriteBytesFromDecomposedInteger(PixelData, BytesPerChannel, RawGreen);

			PixelData += BytesPerChannel;
		}
		if (HasBlueChannel(Format))
		{
			auto RawBlue = static_cast<uint16_t>(Value.B * ScalingCoefficient);
			WriteBytesFromDecomposedInteger(PixelData, BytesPerChannel, RawBlue);

			PixelData += BytesPerChannel;
		}
		if (HasAlphaChannel(Format) || HasPaddingAlphaChannel(Format))
		{
			uint16_t RawAlpha = 0xFFFF;
			// NOTE : if alpha channel is meaningful then store it, otherwise write 0xFF as a stub
			if (HasAlphaChannel(Format))
				RawAlpha = static_cast<uint16_t>(Value.A * ScalingCoefficient);
			WriteBytesFromDecomposedInteger(PixelData, BytesPerChannel, RawAlpha);

			PixelData += BytesPerChannel;
		}
	}
}

uint8_t* Image::GetPointerToFirstByteOfPixel(uint16_t X, uint16_t Y)
{
	return const_cast<uint8_t*>(const_cast<const Image*>(this)->GetPointerToFirstByteOfPixel(X, Y));
}

const uint8_t* Image::GetPointerToFirstByteOfPixel(uint16_t X, uint16_t Y) const
{
	auto Result = Data.data() + (Width * Y + X) * GetBytesPerPixel();

	return Result;
}
