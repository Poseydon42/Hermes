#include <iostream>
#include <fstream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "PNGLoader.h"
#include "stb_image.h"

/*
 * TODO : setup our build so that we can use our own platform layer
 * for file IO etc. instead of using STL
 * Currently we can't do that because our engine builds as one gigantic module
 * and so we have to run full engine startup in order to use platform layer properly,
 * which we don't want obviously.
 */

#ifdef HERMES_PLATFORM_WINDOWS
#define PACKED_STRUCT_BEGIN __pragma(pack(push, 1))
#define PACKED_STRUCT_END   __pragma(pack(pop))
#else
#error "Unsupported platform"
#endif

// NOTE : keep in sync with Hermes' source code until we'll be able to include its headers inside tools sources
enum class AssetType : uint8_t
{
	Image = 1,
	Mesh = 2
};

enum class ImageFormat : uint8_t
{
	R8 = 0x01,
	R16 = 0x02,
	R32 = 0x03,
	R8G8 = 0x05,
	R16G16 = 0x0A,
	B8G8R8X8 = 0x54,
	B8G8R8A8 = 0x55,
	R16G16B16X16 = 0xA8,
	R16G16B16A16 = 0xAA,
	HDR96 = 0xFC
};

PACKED_STRUCT_BEGIN
struct AssetHeader
{
	AssetType Type;
};

struct ImageHeader
{
	uint16_t Width;
	uint16_t Height;
	ImageFormat Format;
};
PACKED_STRUCT_END

std::vector<uint8_t> ReadAllFile(const std::string& Path)
{
	std::vector<uint8_t> Result;
	try
	{
		std::ifstream File = std::ifstream(Path, std::ios::in | std::ios::binary);
		if (!File.is_open())
		{
			return Result;
		}

		File.seekg(0, std::ios::end);
		size_t FileSize = File.tellg();
		File.seekg(0, std::ios::beg);
		Result.resize(FileSize);

		File.read(reinterpret_cast<char*>(Result.data()), static_cast<std::streamsize>(FileSize));
	}
	catch (const std::exception& Error)
	{
		std::cerr << "Caught exception while reading file " << Path << std::endl;
		std::cerr << Error.what() << std::endl;
	}

	return Result;
}

int WriteAssetFile(const char* Data, size_t DataSize, uint16_t Width, uint16_t Height, uint8_t BitsPerChannel,
                   bool IsAlphaChannelAvailable, bool IsMonochrome, bool IsHDR, const std::string& Filename)
{
	if (BitsPerChannel != 8 && BitsPerChannel != 16 && !IsHDR)
	{
		std::cerr << "Channel bit depth other than 8 or 16 is not supported yet" << std::endl;
		return 4;
	}
	ImageFormat Format;
	if (IsMonochrome)
	{
		if (BitsPerChannel == 8)
		{
			if (IsAlphaChannelAvailable)
				Format = ImageFormat::R8G8; // NOTE : on a bit level, R8A8 and R8G8 are encoded in the same way
			else
				Format = ImageFormat::R8;
		}
		else if (BitsPerChannel == 16)
		{
			if (IsAlphaChannelAvailable)
				Format = ImageFormat::R16G16;
			else
				Format = ImageFormat::R16;
		}
	}
	else if (IsAlphaChannelAvailable)
	{
		if (BitsPerChannel == 8)
			Format = ImageFormat::B8G8R8A8;
		else
			Format = ImageFormat::R16G16B16A16;
	}
	else
	{
		if (BitsPerChannel == 8)
			Format = ImageFormat::B8G8R8X8;
		else
			Format = ImageFormat::R16G16B16X16;
	}

	if (IsHDR)
	{
		Format = ImageFormat::HDR96;
	}

	AssetHeader AssetHeader;
	AssetHeader.Type = AssetType::Image;

	ImageHeader ImageHeader;
	ImageHeader.Width = Width;
	ImageHeader.Height = Height;
	ImageHeader.Format = Format;

	try
	{
		std::ofstream OutputFile(Filename, std::ios::binary);

		OutputFile.write(reinterpret_cast<const char*>(&AssetHeader), sizeof(AssetHeader));
		OutputFile.write(reinterpret_cast<const char*>(&ImageHeader), sizeof(ImageHeader));
		OutputFile.write(Data, static_cast<std::streamsize>(DataSize));
	}
	catch (const std::exception& Error)
	{
		std::cerr << "Caught exception while trying to write output file " << Filename << std::endl;
		std::cerr << Error.what();
		return 4;
	}

	return 0;
}

int ConvertFromTGA(const std::string& Path, const std::string& OutputFilename)
{
	auto FileContents = ReadAllFile(Path);

	PACKED_STRUCT_BEGIN
	struct TGAHeader
	{
		uint8_t IDLength;
		uint8_t ColorMapType;
		uint8_t ImageType;
		uint8_t ColorMapSpecificationSkipped[5]; // We're not loading images with color map, so we'll just skip this field and add 5 padding bytes instead
		struct ImageSpecification
		{
			uint16_t XOrigin;
			uint16_t YOrigin;
			uint16_t Width;
			uint16_t Height;
			uint8_t  PixelDepth;
			uint8_t  ImageDescriptor;
		} Specification;
	};

	struct TGAFooter
	{
		uint32_t ExtensionAreaOffset;
		uint32_t DeveloperDirectoryOffset;
		char Signature[18];
	};
	PACKED_STRUCT_END
	auto* InputTGAHeader = reinterpret_cast<TGAHeader*>(FileContents.data());
	auto* InputTGAFooter = reinterpret_cast<TGAFooter*>(FileContents.data() + FileContents.size() - sizeof(TGAFooter));

	bool IsNewFormat = strcmp(InputTGAFooter->Signature, "TRUEVISION-XFILE.") == 0;

	bool IsAlphaChannelAvailable;
	if (IsNewFormat && InputTGAFooter->ExtensionAreaOffset != 0)
	{
		uint8_t* ExtensionArea = FileContents.data() + InputTGAFooter->ExtensionAreaOffset;
		uint8_t AttributesType = *(ExtensionArea + 0x1EE);
		if (AttributesType == 2)
			IsAlphaChannelAvailable = true;
		else
			IsAlphaChannelAvailable = false;
	}
	else
	{
		IsAlphaChannelAvailable = (InputTGAHeader->Specification.ImageDescriptor & 0x0F) != 0;
	}
	bool IsMonochrome = (InputTGAHeader->ImageType & 0x0F) == 3;
	bool IsLeftToRight = !(InputTGAHeader->Specification.ImageDescriptor & 0x10);
	bool IsTopToBottom = InputTGAHeader->Specification.ImageDescriptor & 0x20;

	std::cout << "TGA format: " << (IsNewFormat ? "new" : "old") << std::endl;
	std::cout << "Image size: " << InputTGAHeader->Specification.Width << " x " << InputTGAHeader->Specification.Height << std::endl;
	std::cout << "Image depth: " << static_cast<uint16_t>(InputTGAHeader->Specification.PixelDepth) << " bits per pixel" << std::endl;
	std::cout << "Is monochrome: " << (IsMonochrome ? "true" : "false") << std::endl;
	std::cout << "Alpha channel available: " << (IsAlphaChannelAvailable ? "true" : "false") << std::endl;
	std::cout << "Horizontal direction: " << (IsLeftToRight ? "left-to-right" : "right-to-left") << std::endl;
	std::cout << "Vertical direction: " << (IsTopToBottom ? "top-to-bottom" : "bottom-to-top") << std::endl;

	if (InputTGAHeader->Specification.XOrigin != 0 || InputTGAHeader->Specification.YOrigin != 0)
	{
		std::cerr << "X and Y origin must be zero" << std::endl;
		return 3;
	}
	if ((InputTGAHeader->ImageType & 0x03) == 1)
	{
		std::cerr << "TGA files with color map are currently not supported" << std::endl;
		return 3;
	}
	if (InputTGAHeader->ImageType & 0x08)
	{
		std::cerr << "Compressed TGA files are currently not supported" << std::endl;
		return 3;
	}

	size_t IntermediateBytesPerPixel;
	if (IsMonochrome)
		IntermediateBytesPerPixel = 1;
	else
		IntermediateBytesPerPixel = 4;
	size_t SourceBytesPerPixel = InputTGAHeader->Specification.PixelDepth / 8;

	size_t TotalBytesUsed = static_cast<size_t>(InputTGAHeader->Specification.Width) * InputTGAHeader->Specification.Height * IntermediateBytesPerPixel;

	auto* IntermediateImage = reinterpret_cast<uint8_t*>(malloc(TotalBytesUsed));
	if (!IntermediateImage)
	{
		std::cerr << "Failed to allocate memory for intermediate image representation" << std::endl;
		return 3;
	}

	uint8_t* SourceImagePixel = FileContents.data() + sizeof(TGAHeader) + InputTGAHeader->IDLength;
	// NOTE : Hermes always uses top-to-bottom left-to-right images,
	// so we have to apply flipping if necessary while converting
	for (size_t Y = 0; Y < InputTGAHeader->Specification.Height; Y++)
	{
		for (size_t X = 0; X < InputTGAHeader->Specification.Width; X++)
		{
			uint8_t R = SourceImagePixel[2];
			uint8_t G = SourceImagePixel[1];
			uint8_t B = SourceImagePixel[0];

			size_t ActualX, ActualY;
			if (IsLeftToRight)
				ActualX = X;
			else
				ActualX = InputTGAHeader->Specification.Width - 1 - X;
			if (IsTopToBottom)
				ActualY = Y;
			else
				ActualY = InputTGAHeader->Specification.Height - 1 - Y;

			uint8_t* IntermediateImagePixel = IntermediateImage + (ActualY * InputTGAHeader->Specification.Width + ActualX) * IntermediateBytesPerPixel;

			if (IsMonochrome)
			{
				IntermediateImagePixel[0] = B;
			}
			else
			{
				IntermediateImagePixel[0] = B;
				IntermediateImagePixel[1] = G;
				IntermediateImagePixel[2] = R;
				if (IsAlphaChannelAvailable)
				{
					uint8_t A = SourceImagePixel[3];
					IntermediateImagePixel[3] = A;
				}
				else
				{
					IntermediateImagePixel[3] = 0xFF;
				}
			}

			SourceImagePixel += SourceBytesPerPixel;
		}
	}

	return WriteAssetFile(reinterpret_cast<const char*>(IntermediateImage), TotalBytesUsed,
	                      InputTGAHeader->Specification.Width, InputTGAHeader->Specification.Height,
	                      8, IsAlphaChannelAvailable, IsMonochrome, false,
	                      OutputFilename);
}

int ConvertFromPNG(const std::string& Path, const std::string& OutputFilename)
{
	PNGLoader Image(Path);
	if (!Image.IsValid())
	{
		std::cerr << "Image decompression and/or decoding failed" << std::endl;
		return 3;
	}

	auto Width = Image.GetWidth();
	auto Height = Image.GetHeight();
	size_t PixelCount = static_cast<size_t>(Width) * Height;

	std::cout << "Width: " << Width << std::endl;
	std::cout << "Height: " << Height << std::endl;
	std::cout << "Bits per pixel: " << Image.GetBytesPerPixel() * 8 << std::endl;
	std::cout << "Is monochrome: " << (Image.IsMonochrome() ? "true" : "false") << std::endl;
	std::cout << "Has alpha channel: " << (Image.HasAlphaChannel() ? "true" : "false") << std::endl;

	// NOTE : we need to add alpha channel if necessary and swap B and R channels
	size_t DestBytesPerPixel = Image.GetBytesPerPixel();
	if (!Image.IsMonochrome() && !Image.HasAlphaChannel())
	{
		// NOTE : non-grayscale(e.g. RGB) image must have alpha channel
		DestBytesPerPixel += static_cast<size_t>(Image.GetBitsPerChannel() / 8);
	}

	size_t DestBufferSize = DestBytesPerPixel * PixelCount;
	auto* DestBuffer = static_cast<uint8_t*>(malloc(DestBufferSize));
	if (!DestBuffer)
	{
		std::cerr << "Failed to allocate buffer for image" << std::endl;
		return 4;
	}

	auto* Source = Image.GetPixels();
	auto* Dest = DestBuffer;
	for (auto Scanline = 0; Scanline < Height; Scanline++)
	{
		for (auto X = 0; X < Width; X++)
		{
			if (Image.GetBitsPerChannel() == 8)
			{
				if (Image.IsMonochrome())
				{
					*Dest++ = *Source++;
					if (Image.HasAlphaChannel())
						*Dest++ = *Source++;
				}
				else
				{
					auto R = *Source++;
					auto G = *Source++;
					auto B = *Source++;
					*Dest++ = B;
					*Dest++ = G;
					*Dest++ = R;
					if (Image.HasAlphaChannel())
						*Dest++ = *Source++;
					else
						*Dest++ = 0xFF;
				}
			}
			else
			{
				// NOTE : 16 bits per pixel
				const auto* WordSource = reinterpret_cast<const uint16_t*>(Source);
				auto* WordDest = reinterpret_cast<uint16_t*>(Dest);
				if (Image.IsMonochrome())
				{
					*WordDest++ = *WordSource++;
					if (Image.HasAlphaChannel())
						*WordDest++ = *WordSource++;
				}
				else
				{
					// NOTE : no need for R-B swap in this case
					*WordDest++ = *WordSource++;
					*WordDest++ = *WordSource++;
					*WordDest++ = *WordSource++;
					// Advance original pointers
					Source += 3 * sizeof(*WordSource);
					Dest += 3 * sizeof(*WordDest);
					if (Image.HasAlphaChannel())
					{
						*WordDest++ = *WordSource++;
						Source += sizeof(*WordSource);
					}
					else
					{
						*WordDest++ = 0xFFFF;
					}
					// Advance further for alpha channel
					Dest += sizeof(*WordDest);
				}
			}
		}
	}
	return WriteAssetFile(reinterpret_cast<const char*>(DestBuffer), DestBufferSize, Width, Height,
	                      Image.GetBitsPerChannel(), Image.HasAlphaChannel(), Image.IsMonochrome(), false,
	                      OutputFilename);
}

int ConvertFromHDR(const std::string& Path, const std::string& OutputFilename)
{
	auto FileContents = ReadAllFile(Path);

	if (!stbi_is_hdr_from_memory(FileContents.data(), 
		static_cast<int>(FileContents.size())))
	{
		std::cerr << "File " << Path << " is not a valid HDR Radiance file" << std::endl;
		return 3;
	}

	int Width, Height, NumChannels;
	auto* ImageData = stbi_loadf_from_memory(
		FileContents.data(), static_cast<int>(FileContents.size()),
		&Width, &Height, &NumChannels, 0);

	if (ImageData == nullptr)
	{
		std::cerr << "Failed to load image from file " << Path << std::endl;
		return 3;
	}

	int Result = 0;
	if (NumChannels != 3)
	{
		std::cerr << "File " << Path << " has number of channels " << NumChannels << " other than 3. Loading of this type of file is not supported." << std::endl;
		Result = 3;
	}
	else
	{
		Result = WriteAssetFile(reinterpret_cast<const char*>(ImageData),
		                        static_cast<size_t>(Width) * Height * NumChannels * sizeof(ImageData[0]),
		                        static_cast<uint16_t>(Width), static_cast<uint16_t>(Height),
		                        32, false, false, true,
		                        OutputFilename);
	}

	stbi_image_free(ImageData);
	return Result;
}

/*
 * Usage:
 * imgtoasset <input file> [options]
 * Possible options:
 *  --tga: override file extension and parse it as TGA image
 *	--png: override file extension and parse it as PNG image
 *	--hdr: override file extension and parse it as HDR image
 * Currently supported image formats:
 *  - TGA
 *	- PNG
 *	- HDR(Radiance format)
 */
int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "You need to specify input file" << std::endl;
		return 1;
	}

	enum class FileType
	{
		Undefined,
		TGA,
		PNG,
		HDR
	} FileType = FileType::Undefined;

	std::string InputFilename = std::string(argv[1]);
	std::string InputFileNameWithoutExtension = InputFilename.substr(0, InputFilename.find_last_of('.'));
	for (int ArgumentIndex = 2; ArgumentIndex < argc; ArgumentIndex++)
	{
		if (strcmp(argv[ArgumentIndex], "--tga") == 0)
			FileType = FileType::TGA;
		if (strcmp(argv[ArgumentIndex], "--png") == 0)
			FileType = FileType::PNG;
		if (strcmp(argv[ArgumentIndex], "--hdr") == 0)
			FileType = FileType::HDR;
	}
	if (InputFilename.substr(InputFilename.find_last_of('.'), InputFilename.length() - InputFilename.find_last_of('.') + 1) == ".tga")
	{
		FileType = FileType::TGA;
	}
	if (InputFilename.substr(InputFilename.find_last_of('.'),
	                         InputFilename.length() - InputFilename.find_last_of('.') + 1) == ".png")
	{
		FileType = FileType::PNG;
	}
	if (InputFilename.substr(InputFilename.find_last_of('.'), InputFilename.length() - InputFilename.find_last_of('.') + 1) == ".hdr")
	{
		FileType = FileType::HDR;
	}

	std::string OutputFileName = InputFileNameWithoutExtension + ".hac";

	switch (FileType)
	{
	case FileType::TGA:
		return ConvertFromTGA(InputFilename, OutputFileName);
	case FileType::PNG:
		return ConvertFromPNG(InputFilename, OutputFileName);
	case FileType::HDR:
		return ConvertFromHDR(InputFilename, OutputFileName);
	case FileType::Undefined:
	default:
		std::cerr << "Unknown file format" << std::endl;
		return 2;
	}
}
