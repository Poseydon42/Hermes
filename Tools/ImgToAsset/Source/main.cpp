#include <iostream>
#include <fstream>
#include <vector>

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
	B8G8R8A8 = 0x55
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

int WriteAssetFile(
	const char* Data, size_t DataSize,
	uint16_t Width, uint16_t Height,
	bool IsAlphaChannelAvailable, bool IsMonochrome,
	const std::string& Filename)
{
	ImageFormat Format;
	if (IsMonochrome)
	{
		Format = ImageFormat::R8;
	}
	else if (IsAlphaChannelAvailable)
	{
		Format = ImageFormat::B8G8R8A8;
	}
	else
	{
		Format = ImageFormat::B8G8R8X8;
	}

	AssetHeader AssetHeader;
	AssetHeader.Type = AssetType::Image;

	ImageHeader ImageHeader;
	ImageHeader.Width = Width;
	ImageHeader.Height = Height;
	ImageHeader.Format = Format;

	try
	{
		std::ofstream OutputFile(Filename, std::ios::binary | std::ios::out);

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
	try
	{
		std::ifstream File = std::ifstream(Path, std::ios::in | std::ios::binary);
		if (!File.is_open())
		{
			std::cerr << "Failed to open file " << Path << " for reading" << std::endl;
			return 3;
		}

		File.seekg(0, std::ios::end);
		size_t FileSize = File.tellg();
		File.seekg(0, std::ios::beg);

		auto* FileContents = reinterpret_cast<uint8_t*>(malloc(FileSize));
		if (!FileContents)
		{
			std::cerr << "Failed to allocate memory to read input file" << std::endl;
			return 3;
		}
		File.read(reinterpret_cast<char*>(FileContents), static_cast<std::streamsize>(FileSize));

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
		auto* InputTGAHeader = reinterpret_cast<TGAHeader*>(FileContents);
		auto* InputTGAFooter = reinterpret_cast<TGAFooter*>(FileContents + FileSize - sizeof(TGAFooter));

		bool IsNewFormat = strcmp(InputTGAFooter->Signature, "TRUEVISION-XFILE.") == 0;
		
		bool IsAlphaChannelAvailable;
		if (IsNewFormat && InputTGAFooter->ExtensionAreaOffset != 0)
		{
			uint8_t* ExtensionArea = FileContents + InputTGAFooter->ExtensionAreaOffset;
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
		
		uint8_t* SourceImagePixel = FileContents + sizeof(TGAHeader) + InputTGAHeader->IDLength;
		// NOTE : Hermes always uses top-to-bottom left-to-right images,
		// so we have to apply flipping if necessary while converting
		for (size_t Y = 0; Y < InputTGAHeader->Specification.Height; Y++)
		{
			for (size_t X = 0; X < InputTGAHeader->Specification.Width; X++)
			{
				uint8_t R = SourceImagePixel[2];
				uint8_t G = SourceImagePixel[1];
				uint8_t B = SourceImagePixel[0];
				uint8_t A = SourceImagePixel[3];

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
					IntermediateImagePixel[3] = IsAlphaChannelAvailable ? A : 0xFF;
				}

				SourceImagePixel += SourceBytesPerPixel;
			}
		}

		return WriteAssetFile(
			reinterpret_cast<const char*>(IntermediateImage), TotalBytesUsed,
			InputTGAHeader->Specification.Width, InputTGAHeader->Specification.Height, 
			IsAlphaChannelAvailable, IsMonochrome,
			OutputFilename);
	}
	catch (const std::exception& Error)
	{
		std::cerr << "Caught exception while processing TGA file " << Path << std::endl;
		std::cerr << Error.what() << std::endl;
		return 4;
	}
}

/*
 * Usage:
 * imgtoasset <input file> [options]
 * Possible options:
 *  --tga: override file extension and parse it as TGA image
 * Currently supported image formats:
 *  - TGA
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
		TGA
	} FileType = FileType::Undefined;

	std::string InputFilename = std::string(argv[1]);
	std::string InputFileNameWithoutExtension = InputFilename.substr(0, InputFilename.find_last_of('.'));
	for (int ArgumentIndex = 2; ArgumentIndex < argc; ArgumentIndex++)
	{
		if (strcmp(argv[ArgumentIndex], "--tga") == 0)
			FileType = FileType::TGA;
	}
	if (InputFilename.substr(InputFilename.find_last_of('.'), InputFilename.length() - InputFilename.find_last_of('.') + 1) == ".tga")
	{
		FileType = FileType::TGA;
	}

	std::string OutputFileName = InputFileNameWithoutExtension + ".hac";

	switch (FileType)
	{
	case FileType::TGA:
		return ConvertFromTGA(InputFilename, OutputFileName);
	case FileType::Undefined:
	default:
		std::cerr << "Unknown file format" << std::endl;
		return 2;
	}
}
