#include "PNGLoader.h"

#include <fstream>
#include <iostream>

template<typename T>
static T Abs(T Value)
{
	return Value < 0 ? -Value : Value;
}

/*
 * NOTE : implementation of https://www.w3.org/TR/2003/REC-PNG-20031110/#9Filter-type-4-Paeth
 */
static uint8_t PaethPredictor(int16_t A, int16_t B, int16_t C)
{
	auto P = static_cast<int16_t>(A + B - C);
	auto Pa = Abs(static_cast<int16_t>(P - A));
	auto Pb = Abs(static_cast<int16_t>(P - B));
	auto Pc = Abs(static_cast<int16_t>(P - C));
	int16_t Pr = 0;
	if (Pa <= Pb && Pa <= Pc)
		Pr = A;
	else if (Pb <= Pc)
		Pr = B;
	else
		Pr = C;
	return static_cast<uint8_t>(Pr);
}

PNGLoader::PNGLoader(const std::string& Path)
{
	std::basic_ifstream<uint8_t> File(Path, std::ios::binary | std::ios::ate);
	if (!File.is_open())
	{
		std::cerr << "Cannot open file " << Path << std::endl;
		return;
	}

	size_t FileSize = File.tellg();
	File.seekg(0, std::ios::beg);
	std::vector<uint8_t> FileContents(FileSize);
	File.read(FileContents.data(), static_cast<std::streamsize>(FileSize));

	const uint8_t Signature[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
	if (FileSize < 8 || memcmp(Signature, FileContents.data(), sizeof(Signature)) != 0)
	{
		std::cerr << "File " << Path << " is not a valid PNG file: incorrect signature" << std::endl;
		return;
	}

	size_t NextByte = 8; // Skipped 8 bytes of signature
	std::vector<uint8_t> ChunkContents;
	bool WasIHDRChunkProcessed = false;
	DecompressionStream = {};
	if (inflateInit(&DecompressionStream) != Z_OK)
	{
		std::cerr << "zlib initialization failed; data cannot be processed" << std::endl;
		return;
	}
	while (NextByte < FileSize)
	{
		char ChunkName[5];
		if (!ReadChunk(ChunkContents, ChunkName, FileContents, NextByte))
		{
			std::cerr << "Failed to read next chunk; " << FileSize - NextByte << " bytes left unread" << std::endl;
			return;
		}

		if (strcmp(ChunkName, "IEND") == 0 && NextByte < FileSize)
		{
			std::cerr << "File " << Path << " is invalid PNG file: IEND is not the last chunk" << std::endl;
			break;
		}
		if (strcmp(ChunkName, "IHDR") != 0 && !WasIHDRChunkProcessed)
		{
			std::cerr << "File " << Path << " is invalid PNG file: no IHDR header present" << std::endl;
			break;
		}

		if (strcmp(ChunkName, "IHDR") == 0)
		{
			if (!ProcessIHDRChunk(ChunkContents))
			{
				std::cerr << "Failed to process IHDR chunk" << std::endl;
				break;
			}
			WasIHDRChunkProcessed = true;
		}
		if (strcmp(ChunkName, "IDAT") == 0)
		{
			if (!ProcessIDATChunk(ChunkContents))
			{
				std::cerr << "Failed to process IDAT chunk" << std::endl;
				break;
			}
		}
	}
	inflateEnd(&DecompressionStream);

	ApplyFilterRecovery();
	UnpackPixels();
}

bool PNGLoader::IsValid() const
{
	return Width > 0 && Height > 0 && !Pixels.empty() && GetBytesPerPixel() > 0;
}

bool PNGLoader::HasAlphaChannel() const
{
	return ColorType == ColorType::GrayscaleAlpha || ColorType == ColorType::RGBA;
}

bool PNGLoader::IsMonochrome() const
{
	return ColorType == ColorType::Grayscale || ColorType == ColorType::GrayscaleAlpha;
}

uint16_t PNGLoader::GetWidth() const
{
	return Width;
}

uint16_t PNGLoader::GetHeight() const
{
	return Height;
}

uint8_t PNGLoader::GetBitsPerChannel() const
{
	return std::max<uint8_t>(BitsPerChannel, 8);
}

uint8_t PNGLoader::GetBytesPerPixel() const
{
	uint8_t BytesPerChannel = std::max<uint8_t>(8, BitsPerChannel) / 8;
	switch (ColorType)
	{
	case ColorType::Grayscale:
		return BytesPerChannel;
	case ColorType::GrayscaleAlpha:
		return 2 * BytesPerChannel;
	case ColorType::RGB:
		return 3 * BytesPerChannel;
	case ColorType::RGBA:
		return 4 * BytesPerChannel;
	default:
		std::cerr << "Undefined or unsupported PNG color type" << std::endl;
		return 0;
	}
}

const uint8_t* PNGLoader::GetPixels() const
{
	return Pixels.data();
}

bool PNGLoader::ReadChunk(std::vector<uint8_t>& Result, char ChunkName[5], const std::vector<uint8_t>& Data, size_t& NextByte) const
{
	if (Data.size() - NextByte < 4)
	{
		std::cerr << "Chunk size cannot be read: number of remaining bytes is less than 4" << std::endl;
		return false;
	}
	uint32_t ChunkSize = ReadBigEndianNumber(Data.data() + NextByte);
	NextByte += 4;

	static constexpr size_t ChunkNameLength = 4;
	if (Data.size() - NextByte < 4)
	{
		std::cerr << "Chunk name cannot be read: number of remaining bytes is less than 4" << std::endl;
		return false;
	}
	memcpy(ChunkName, Data.data() + NextByte, ChunkNameLength);
	ChunkName[4] = 0x00;
	NextByte += 4;

	if (Data.size() - NextByte < ChunkSize)
	{
		std::cerr << "Chunk contents cannot be read: number of remaining bytes is less than chunk size" << std::endl;
		return false;
	}
	Result.resize(ChunkSize);
	memcpy(Result.data(), Data.data() + NextByte, ChunkSize);
	NextByte += ChunkSize;

	if (Data.size() - NextByte < 4)
	{
		std::cerr << "Chunk CRC cannot be read: number of remaining bytes is less than 4" << std::endl;
		return false;
	}
	// TODO : implement CRC check
	uint32_t CRC = ReadBigEndianNumber(Data.data() + NextByte);
	(void)CRC;
	NextByte += 4;

	return true;
}

bool PNGLoader::ProcessIHDRChunk(const std::vector<uint8_t>& Data)
{
	static constexpr size_t IHDRChunkSize = 13;
	if (Data.size() != IHDRChunkSize)
	{
		std::cerr << "File is not a valid PNG image: IHDR chunk size is not equal to 13" << std::endl;
		return false;
	}

	uint32_t LocalWidth = ReadBigEndianNumber(Data.data());
	uint32_t LocalHeight = ReadBigEndianNumber(Data.data() + 4);

	static constexpr size_t MaxDimension = std::numeric_limits<uint16_t>::max();
	if (LocalWidth > MaxDimension || LocalHeight > MaxDimension)
	{
		std::cerr << "One of the image dimensions is larger than " << MaxDimension <<
			"; images of such size are not supported" << std::endl;
		return false;
	}

	Width = static_cast<uint16_t>(LocalWidth);
	Height = static_cast<uint16_t>(LocalHeight);

	BitsPerChannel = Data[8];
	ColorType = static_cast<enum ColorType>(Data[9]);
	uint8_t InterlaceMethod = Data[12];
	
	if (ColorType == ColorType::Indexed)
	{
		std::cerr << "Indexed PNG files are not supported; file cannot be read" << std::endl;
		return false;
	}
	if (InterlaceMethod != 0)
	{
		std::cerr << "PNG interlacing is not supported; file cannot be read" << std::endl;
		return false;
	}

	return true;
}

bool PNGLoader::ProcessIDATChunk(const std::vector<uint8_t>& Data)
{
	const auto* OnePastLastDataByte = Data.data() + Data.size();

	DecompressionStream.avail_in = static_cast<uInt>(OnePastLastDataByte - Data.data());
	DecompressionStream.next_in = const_cast<uint8_t*>(Data.data());
	
	do
	{
		static constexpr size_t ChunkSize = 8192;
		uint8_t ChunkBuffer[ChunkSize];

		DecompressionStream.avail_out = ChunkSize;
		DecompressionStream.next_out = ChunkBuffer;

		auto Result = inflate(&DecompressionStream, Z_NO_FLUSH);
		if (Result == Z_STREAM_ERROR)
		{
			std::cerr << "Decompression error; image cannot be processed" << std::endl;
			return false;
		}

		size_t BytesDecompressed = ChunkSize - DecompressionStream.avail_out;
		UncompressedData.insert(UncompressedData.end(), ChunkBuffer, ChunkBuffer + BytesDecompressed);
	} while (DecompressionStream.avail_in > 0);
	return true;
}

uint32_t PNGLoader::ReadBigEndianNumber(const uint8_t* Memory) const
{
	uint32_t Result = Memory[0] << 24 | Memory[1] << 16 | Memory[2] << 8 | Memory[3];
	return Result;
}

void PNGLoader::ApplyFilterRecovery()
{
	size_t Stride;
	if (BitsPerChannel < 8)
		Stride = (Width * BitsPerChannel + 7) / 8;
	else
		Stride = static_cast<size_t>(Width) * GetBytesPerPixel();
	size_t DestArraySize = Stride * Height;
	FilteredData.resize(DestArraySize);
	
	for (uint16_t Scanline = 0; Scanline < Height; Scanline++)
	{
		// NOTE : first byte indicates filtering method applied to this scanline
		const uint8_t* Source = UncompressedData.data() + Scanline * (Stride + 1);
		uint8_t* Dest = FilteredData.data() + Scanline * Stride;

		auto Method = static_cast<FilteringMethod>(*Source++);
		for (size_t ByteIndex = 0; ByteIndex < Stride; ByteIndex++)
		{
			uint8_t ValueOfPreviousCorrespondingByte = 0;
			if (BitsPerChannel >= 8)
			{
				if (ByteIndex >= GetBytesPerPixel())
					ValueOfPreviousCorrespondingByte = Dest[-GetBytesPerPixel()];
			}
			else
			{
				if (ByteIndex > 0)
					ValueOfPreviousCorrespondingByte = Dest[-1];
			}
			uint8_t ValueOfCorrespondingByteInPreviousScanline = 0;
			if (Scanline > 0)
				ValueOfCorrespondingByteInPreviousScanline = Dest[-Stride];
			uint8_t ValueOfPreviousCorrespondingByteInPreviousScanline = 0;
			if (BitsPerChannel >= 8)
			{
				if (Scanline > 0 && ByteIndex >= GetBytesPerPixel())
					ValueOfPreviousCorrespondingByteInPreviousScanline = Dest[-Stride - GetBytesPerPixel()];
			}
			else
			{
				if (Scanline > 0 && ByteIndex > 0)
					ValueOfPreviousCorrespondingByteInPreviousScanline = Dest[-Stride - 1];
			}
					
			switch (Method)
			{
			case FilteringMethod::None:
				*Dest++ = *Source++;
				break;
			case FilteringMethod::Sub:
				*Dest++ = *Source++ + ValueOfPreviousCorrespondingByte;
				break;
			case FilteringMethod::Up:
				*Dest++ = *Source++ + ValueOfCorrespondingByteInPreviousScanline;
				break;
			case FilteringMethod::Average:
				{
					auto FilteredValue = *Source++;
					auto Average = (static_cast<uint16_t>(ValueOfPreviousCorrespondingByte) +
						ValueOfCorrespondingByteInPreviousScanline) / 2;
					*Dest++ = FilteredValue + static_cast<uint8_t>(Average);
					break;
				}
			case FilteringMethod::Paeth:
				{
					auto FilteredValue = *Source++;
					*Dest++ = FilteredValue + PaethPredictor(ValueOfPreviousCorrespondingByte,
					                                         ValueOfCorrespondingByteInPreviousScanline,
					                                         ValueOfPreviousCorrespondingByteInPreviousScanline);
					break;
				}
			}
		}
	}
}

void PNGLoader::UnpackPixels()
{
	if (BitsPerChannel == 8)
	{
		Pixels = FilteredData;
		return;
	}

	size_t PixelArraySize = static_cast<size_t>(Width) * Height * GetBytesPerPixel();
	Pixels.resize(PixelArraySize);

	const auto* Source = FilteredData.data();
	auto* Dest = Pixels.data();

	if (BitsPerChannel == 16)
	{
		// NOTE : in this case the only thing we need to do is swap endianness of PixelArraySize / 2 values
		for (size_t Index = 0; Index < PixelArraySize / 2; Index++)
		{
			uint8_t High = *Source++;
			uint8_t Low = *Source++;
			*Dest++ = Low;
			*Dest++ = High;
		}
	}
	else
	{
		uint16_t PixelsPerPackedValue = 8 / BitsPerChannel;
		for (size_t Y = 0; Y < Height; Y++)
		{
			auto PixelsLeftInCurrentScanline = Width;
			size_t BytesInCurrentScanline = (static_cast<size_t>(Width) + PixelsPerPackedValue - 1) /
				PixelsPerPackedValue * GetBytesPerPixel();
			for (size_t X = 0; X < std::min<size_t>(BytesInCurrentScanline, Width); X++)
			{
				uint8_t Value = *Source++;

				for (uint8_t PackedPixelIndex = 0; PackedPixelIndex < std::min(PixelsLeftInCurrentScanline, PixelsPerPackedValue); PackedPixelIndex++)
				{
					uint8_t Shift = 8 - (PackedPixelIndex + 1) * BitsPerChannel;
					uint8_t Mask = 0;
					for (auto Bit = 0; Bit < BitsPerChannel; Bit++)
						Mask += 1 << Bit;
					uint8_t UnpackedValue = ((Value >> Shift) & Mask);

					// NOTE : unpacked value is stored in lower bits of the variable and needs to be
					//        repeatedly copied to keep values proportional
					if (BitsPerChannel == 1)
					{
						UnpackedValue = static_cast<uint8_t>(UnpackedValue | (UnpackedValue << 1) | (UnpackedValue << 2)
							| (UnpackedValue << 3) | (UnpackedValue << 4) | (UnpackedValue << 5) | (UnpackedValue << 6)
							| (UnpackedValue << 7));
					}
					else if (BitsPerChannel == 2)
					{
						UnpackedValue = static_cast<uint8_t>(UnpackedValue | (UnpackedValue << 2) | (UnpackedValue << 4)
							| (UnpackedValue << 6));
					}
					else if (BitsPerChannel == 4)
					{
						UnpackedValue = static_cast<uint8_t>(UnpackedValue | (UnpackedValue << 4));
					}
					*Dest++ = UnpackedValue;
				}
				PixelsLeftInCurrentScanline -= PixelsPerPackedValue;
			}
		}
	}
}
