#include "AssetLoader.h"

#include "Platform/GenericPlatform/PlatformFile.h"
#include "AssetSystem/ImageAsset.h"
#include "AssetSystem/MeshAsset.h"
#include "Logging/Logger.h"
#include "VirtualFilesystem/VirtualFilesystem.h"

namespace Hermes
{
	std::unique_ptr<Asset> AssetLoader::Load(StringView Name)
	{
		String Filename = String(Name) + ".hac";

		auto File = VirtualFilesystem::Open(Filename, FileOpenMode::OpenExisting, FileAccessMode::Read);
		if (!File)
		{
			HERMES_LOG_WARNING("Failed to open asset file %s", Name.data());
			return nullptr;
		}

		AssetHeader Header = {};
		if (!File->Read(&Header, sizeof(Header)))
		{
			HERMES_LOG_WARNING("Failed to read asset header from asset file %s", Name.data());
			return nullptr;
		}

		switch (Header.Type)
		{
		case AssetType::Image:
			return LoadImage(*File, Header, Name);
		case AssetType::Mesh:
			return LoadMesh(*File, Header, Name);
		default:
			HERMES_LOG_WARNING("Loading assets of type %02x is currently unsupported or it is unknown type", static_cast<uint8>(Header.Type));
			return nullptr;
		}
	}

	std::unique_ptr<Asset> AssetLoader::LoadImage(IPlatformFile& File, const AssetHeader&, StringView Name)
	{
		ImageAssetHeader Header = {};
		if (!File.Read(&Header, sizeof(Header)))
		{
			HERMES_LOG_WARNING("Failed to read image header from asset %s", Name.data());
			return nullptr;
		}

		size_t BytesPerPixel = NumberOfChannelInImageFormat(Header.Format) * Header.BytesPerChannel;
		size_t TotalBytes = CalculateTotalPixelCount(Header.Width, Header.Height, Header.MipLevelCount) * BytesPerPixel;
		std::vector<uint8> ImageData(TotalBytes, 0x00);
		if (!File.Read(ImageData.data(), TotalBytes))
		{
			HERMES_LOG_WARNING("Failed to read image data from asset %s", Name.data());
			return nullptr;
		}

		auto Result = std::unique_ptr<ImageAsset>(new ImageAsset(String(Name), Vec2ui{Header.Width, Header.Height},
		                                                         Header.Format, Header.BytesPerChannel, Header.MipLevelCount,
		                                                         ImageData.data()));

		return Result;
	}

	std::unique_ptr<Asset> AssetLoader::LoadMesh(IPlatformFile& File, const AssetHeader& AssetHeader, StringView Name)
	{
		(void)AssetHeader;

		MeshAssetHeader Header = {};
		if (!File.Read(&Header, sizeof(Header)))
		{
			HERMES_LOG_WARNING("Failed to read mesh header from asset %s", Name.data());
			return nullptr;
		}

		std::vector<Vertex> Vertices(Header.VertexCount);
		std::vector<uint32> Indices(Header.IndexCount);
		if (!File.Read(Vertices.data(), Header.VertexCount * sizeof(Vertex)) ||
			!File.Read(Indices.data(), Header.IndexCount * sizeof(uint32)))
		{
			HERMES_LOG_WARNING("Failed to read mesh data from asset %s", Name.data());
			return nullptr;
		}

		return std::unique_ptr<MeshAsset>(new MeshAsset(String(Name), Vertices, Indices));
	}
}
