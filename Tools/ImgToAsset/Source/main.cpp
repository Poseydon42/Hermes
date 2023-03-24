#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

#include "Core/Misc/ArgsParser.h"

#define STB_IMAGE_IMPLEMENTATION

#include "Image.h"
#include "PNGLoader.h"
#include "stb_image.h"

#include "Math/Math.h"

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

static Hermes::ImageFormat ChooseImageFormat(bool IsMonochrome, bool HasAlphaChannel)
{
	if (IsMonochrome)
	{
		if (HasAlphaChannel)
			return Hermes::ImageFormat::RA;
		else
			return Hermes::ImageFormat::R;
	}
	else
	{
		if (HasAlphaChannel)
			return Hermes::ImageFormat::RGBA;
		else
			return Hermes::ImageFormat::RGBX;
	}
}

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

int WriteAssetFile(const std::vector<const Image*>& Images, const std::string& Filename)
{
	Hermes::AssetHeader AssetHeader = {};
	memcpy(AssetHeader.Signature, Hermes::AssetHeader::ExpectedSignature, sizeof(AssetHeader.Signature));
	AssetHeader.Type = Hermes::AssetType::Texture2D;

	Hermes::ImageAssetHeader ImageHeader = {};
	ImageHeader.Width = Images[0]->GetWidth();
	ImageHeader.Height = Images[0]->GetHeight();
	ImageHeader.Format = Images[0]->GetFormat();
	ImageHeader.BytesPerChannel = static_cast<uint8_t>(Images[0]->GetBytesPerChannel());
	ImageHeader.MipLevelCount = static_cast<uint8_t>(Images.size());

	try
	{
		std::ofstream OutputFile(Filename, std::ios::binary);

		OutputFile.write(reinterpret_cast<const char*>(&AssetHeader), sizeof(AssetHeader));
		OutputFile.write(reinterpret_cast<const char*>(&ImageHeader), sizeof(ImageHeader));
		for (const auto* Image : Images)
		{
			OutputFile.write(static_cast<const char*>(Image->GetData()),
			                 static_cast<std::streamsize>(Image->GetDataSize()));
		}		
	}
	catch (const std::exception& Error)
	{
		std::cerr << "Caught exception while trying to write output file " << Filename << std::endl;
		std::cerr << Error.what();
		return 4;
	}

	return 0;
}

std::unique_ptr<Image> ReadTGA(const std::string& Path)
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
		return nullptr;
	}
	if ((InputTGAHeader->ImageType & 0x03) == 1)
	{
		std::cerr << "TGA files with color map are currently not supported" << std::endl;
		return nullptr;
	}
	if (InputTGAHeader->ImageType & 0x08)
	{
		std::cerr << "Compressed TGA files are currently not supported" << std::endl;
		return nullptr;
	}

	Hermes::ImageFormat Format = ChooseImageFormat(IsMonochrome, IsAlphaChannelAvailable);
	// NOTE : TGAs are always 1 byte per channel
	auto Result = std::make_unique<Image>(InputTGAHeader->Specification.Width, InputTGAHeader->Specification.Height,
	                                      Format, 1);

	uint8_t* SourceImagePixel = FileContents.data() + sizeof(TGAHeader) + InputTGAHeader->IDLength;
	size_t SourceBytesPerPixel = InputTGAHeader->Specification.PixelDepth / 8;
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

			uint8_t* IntermediateImagePixel = static_cast<uint8_t*>(Result->GetData()) + (ActualY * InputTGAHeader->
				Specification.Width + ActualX) * Result->GetBytesPerPixel();

			if (IsMonochrome)
			{
				IntermediateImagePixel[0] = B;
			}
			else
			{
				IntermediateImagePixel[0] = R;
				IntermediateImagePixel[1] = G;
				IntermediateImagePixel[2] = B;
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

	return Result;
}

std::unique_ptr<Image> ReadPNG(const std::string& Path)
{
	PNGLoader PNGImage(Path);
	if (!PNGImage.IsValid())
	{
		std::cerr << "Image decompression and/or decoding failed" << std::endl;
		return nullptr;
	}

	auto Width = PNGImage.GetWidth();
	auto Height = PNGImage.GetHeight();

	std::cout << "Width: " << Width << std::endl;
	std::cout << "Height: " << Height << std::endl;
	std::cout << "Bits per pixel: " << PNGImage.GetBytesPerPixel() * 8 << std::endl;
	std::cout << "Is monochrome: " << (PNGImage.IsMonochrome() ? "true" : "false") << std::endl;
	std::cout << "Has alpha channel: " << (PNGImage.HasAlphaChannel() ? "true" : "false") << std::endl;

	// NOTE : we need to add alpha channel if necessary
	Hermes::ImageFormat DestFormat = ChooseImageFormat(PNGImage.IsMonochrome(), PNGImage.HasAlphaChannel());
	size_t BytesPerChannel = (PNGImage.GetBitsPerChannel() == 16 ? 2 : 1);
	auto Result = std::make_unique<Image>(PNGImage.GetWidth(), PNGImage.GetHeight(), DestFormat, BytesPerChannel);
	auto* Source = PNGImage.GetPixels();
	auto* Dest = static_cast<uint8_t*>(Result->GetData());
	for (auto Scanline = 0; Scanline < Height; Scanline++)
	{
		for (auto X = 0; X < Width; X++)
		{
			if (PNGImage.GetBitsPerChannel() == 8)
			{
				if (PNGImage.IsMonochrome())
				{
					*Dest++ = *Source++;
					if (PNGImage.HasAlphaChannel())
						*Dest++ = *Source++;
				}
				else
				{
					// NOTE : copying red, green and blue channels
					*Dest++ = *Source++;
					*Dest++ = *Source++;
					*Dest++ = *Source++;
					if (PNGImage.HasAlphaChannel())
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
				if (PNGImage.IsMonochrome())
				{
					*WordDest++ = *WordSource++;
					if (PNGImage.HasAlphaChannel())
						*WordDest++ = *WordSource++;
				}
				else
				{
					*WordDest++ = *WordSource++;
					*WordDest++ = *WordSource++;
					*WordDest++ = *WordSource++;
					// Advance original pointers
					Source += 3 * sizeof(*WordSource);
					Dest += 3 * sizeof(*WordDest);
					if (PNGImage.HasAlphaChannel())
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
	return Result;
}

std::unique_ptr<Image> ReadHDR(const std::string& Path)
{
	auto FileContents = ReadAllFile(Path);

	if (!stbi_is_hdr_from_memory(FileContents.data(), 
		static_cast<int>(FileContents.size())))
	{
		std::cerr << "File " << Path << " is not a valid HDR Radiance file" << std::endl;
		return nullptr;
	}

	int Width, Height, NumChannels;
	auto* ImageData = stbi_loadf_from_memory(
		FileContents.data(), static_cast<int>(FileContents.size()),
		&Width, &Height, &NumChannels, 0);

	if (ImageData == nullptr)
	{
		std::cerr << "Failed to load image from file " << Path << std::endl;
		return nullptr;
	}
	
	if (NumChannels != 3)
	{
		std::cerr << "File " << Path << " has number of channels " << NumChannels << " other than 3. Loading of this type of file is not supported." << std::endl;
		return nullptr;
	}

	// NOTE : unfortunately, STB does not allow to load image into a preallocated buffer,
	//        so we'll have to do one more copy here
	auto Result = std::make_unique<Image>(static_cast<uint16_t>(Width), static_cast<uint16_t>(Height), Hermes::ImageFormat::HDR,
	                                      4, ImageData);
	stbi_image_free(ImageData);
	return Result;
}

void DisplayHelpMessage()
{
	constexpr char UsageMessage[] = 
		"Usage:\n"
		"    imgtoasset <input file> [options]\n"
		"Possible options:\n"
		"    -h, --help: display this help message\n"
		"    -m, --generate-mips: generate mip maps and store them in the converted asset"
		"Currently supported image formats:\n"
		"    - TGA\n"
		"    - PNG\n"
		"    - HDR(Radiance format)";

	std::cout << UsageMessage << std::endl;
}

int main(int ArgCount, const char** ArgValues)
{
	if (ArgCount < 2)
	{
		std::cerr << "Missing input file; use --help option for more information" << std::endl;
		return 1;
	}

	bool DisplayHelp = false;
	bool GenerateMips = false;
	Hermes::String InputFileName;

	Hermes::ArgsParser Args;
	Args.AddOption("help", 'h', &DisplayHelp);
	Args.AddOption("generate-mips", 'm', &GenerateMips);
	Args.AddPositional(true, &InputFileName);

	bool ParsingSuccess = Args.Parse(ArgCount - 1, ArgValues + 1); // Skipping first argument as it's the name of the application
	if (DisplayHelp)
	{
		DisplayHelpMessage();
		return 0;
	}
	if (!ParsingSuccess || InputFileName.empty())
	{
		std::cerr << "Invalid usage; use --help option for more information" << std::endl;
		return 1;
	}

	enum class FileType
	{
		Undefined,
		TGA,
		PNG,
		HDR
	} FileType = FileType::Undefined;
	
	std::string InputFileNameWithoutExtension = InputFileName.substr(0, InputFileName.find_last_of('.'));
	if (InputFileName.substr(InputFileName.find_last_of('.'),
	                         InputFileName.length() - InputFileName.find_last_of('.') + 1) == ".tga")
	{
		FileType = FileType::TGA;
	}
	if (InputFileName.substr(InputFileName.find_last_of('.'),
	                         InputFileName.length() - InputFileName.find_last_of('.') + 1) == ".png")
	{
		FileType = FileType::PNG;
	}
	if (InputFileName.substr(InputFileName.find_last_of('.'),
	                         InputFileName.length() - InputFileName.find_last_of('.') + 1) == ".hdr")
	{
		FileType = FileType::HDR;
	}
	auto OutputFileName = InputFileNameWithoutExtension + ".hac";

	std::unique_ptr<Image> LoadedImage;
	switch (FileType)
	{
	case FileType::TGA:
		LoadedImage = ReadTGA(InputFileName);
		break;
	case FileType::PNG:
		LoadedImage = ReadPNG(InputFileName);
		break;
	case FileType::HDR:
		LoadedImage = ReadHDR(InputFileName);
		break;
	case FileType::Undefined:
	default:
		std::cerr << "Unknown file format" << std::endl;
		return 2;
	}

	if (!LoadedImage)
	{
		std::cerr << "Cannot read input file " << InputFileName << std::endl;
		return 3;
	}

	std::vector<std::unique_ptr<Image>> Mips;
	Mips.push_back(std::move(LoadedImage));
	if (GenerateMips)
	{
		size_t SourceMip = 0;
		uint16_t NextMipWidth = Mips[0]->GetWidth();
		uint16_t NextMipHeight = Mips[0]->GetHeight();

		while (NextMipWidth > 1 || NextMipHeight > 1)
		{
			NextMipWidth = Hermes::Math::Max<uint16_t>(NextMipWidth / 2, 1);
			NextMipHeight = Hermes::Math::Max<uint16_t>(NextMipHeight / 2, 1);

			auto Mip = Image::CreateDownscaled(*Mips[SourceMip], NextMipWidth, NextMipHeight);

			Mips.push_back(std::move(Mip));

			SourceMip++;
		}
	}

	std::vector<const Image*> Result;
	Result.reserve(Mips.size());
	for (auto& Mip : Mips)
	{
		Result.push_back(Mip.get());
	}
	WriteAssetFile(Result, OutputFileName);
}
