#include "AssetLoader.h"

#include "Platform/GenericPlatform/PlatformFile.h"
#include "AssetSystem/ImageAsset.h"

namespace Hermes
{
	std::shared_ptr<Asset> AssetLoader::Load(const String& Name)
	{
		String Filename = Name + L".hac";

		auto File = PlatformFilesystem::OpenFile(Filename, IPlatformFile::FileAccessMode::Read, IPlatformFile::FileOpenMode::OpenExisting);
		if (!File->IsValid())
		{
			HERMES_LOG_WARNING(L"Failed to open asset file %s", Name.c_str());
			return nullptr;
		}

		AssetHeader Header;
		if (!File->Read(reinterpret_cast<uint8*>(&Header), sizeof(Header)))
		{
			HERMES_LOG_WARNING(L"Failed to read asset header from asset file %s", Name.c_str());
			return nullptr;
		}

		switch (Header.Type)
		{
		case AssetType::Image:
			return LoadImage(*File, Header, Name);
		default:
			HERMES_LOG_WARNING(L"Loading assets of type %02x is currently unsupported or it is unknown type", static_cast<uint8>(Header.Type));
			return nullptr;
		}
	}

	std::shared_ptr<Asset> AssetLoader::LoadImage(IPlatformFile& File, const AssetHeader&, const String& Name)
	{
		PACKED_STRUCT_BEGIN
		struct ImageHeader
		{
			uint16 Width;
			uint16 Height;
			ImageFormat Format;
		};
		PACKED_STRUCT_END

		ImageHeader Header;
		if (!File.Read(reinterpret_cast<uint8*>(&Header), sizeof(ImageHeader)))
		{
			HERMES_LOG_WARNING(L"Failed to read image header from asset %s", Name.c_str());
			return nullptr;
		}

		uint32 BytesPerPixel = BytesPerPixelForImageFormat(Header.Format);
		size_t TotalBytes = static_cast<size_t>(Header.Width) * Header.Height * BytesPerPixel;
		std::vector<uint8> ImageData(TotalBytes, 0x00);
		if (!File.Read(ImageData.data(), TotalBytes))
		{
			HERMES_LOG_WARNING(L"Failed to read image data from asset %s", Name.c_str());
			return nullptr;
		}

		auto Result = std::shared_ptr<ImageAsset>(new ImageAsset(Name, Vec2ui{ Header.Width, Header.Height }, Header.Format, ImageData.data()));

		return Result;
	}
}
