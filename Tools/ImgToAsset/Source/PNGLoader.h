#pragma once

#include <string>
#include <vector>
#include <zlib.h>

class PNGLoader
{
public:
	PNGLoader(const std::string& Path);

	bool IsValid() const;

	bool HasAlphaChannel() const;

	bool IsMonochrome() const;

	uint16_t GetWidth() const;
	uint16_t GetHeight() const;

	/*
	 * NOTE : this function returns bits per channel in the loaded and processed image(e.g. possible values are 8 and 16)
	 */
	uint8_t GetBitsPerChannel() const;

	uint8_t GetBytesPerPixel() const;

	const uint8_t* GetPixels() const;

private:
	uint16_t Width = 0;
	uint16_t Height = 0;
	enum class ColorType : uint8_t
	{
		Grayscale = 0,
		RGB = 2,
		Indexed = 3,
		GrayscaleAlpha = 4,
		RGBA = 6
	} ColorType = ColorType::RGB;
	uint8_t BitsPerChannel = 0;
	std::vector<uint8_t> Pixels;

	z_stream DecompressionStream = {};
	std::vector<uint8_t> UncompressedData;
	std::vector<uint8_t> FilteredData;

	enum class FilteringMethod : uint8_t
	{
		None,
		Sub,
		Up,
		Average,
		Paeth
	};

	bool ReadChunk(std::vector<uint8_t>& Result, char ChunkName[5], const std::vector<uint8_t>& Data, size_t& NextByte) const;
	bool ProcessIHDRChunk(const std::vector<uint8_t>& Data);
	bool ProcessIDATChunk(const std::vector<uint8_t>& Data);
	uint32_t ReadBigEndianNumber(const uint8_t* Memory) const;
	void ApplyFilterRecovery();
	void UnpackPixels();
};
