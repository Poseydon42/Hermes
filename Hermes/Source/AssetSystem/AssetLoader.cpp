﻿#include "AssetLoader.h"

#include "Platform/GenericPlatform/PlatformFile.h"
#include "AssetSystem/ImageAsset.h"
#include "AssetSystem/MeshAsset.h"

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
		case AssetType::Mesh:
			return LoadMesh(*File, Header, Name);
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
			uint8 BytesPerChannel;
		};
		PACKED_STRUCT_END

		ImageHeader Header;
		if (!File.Read(reinterpret_cast<uint8*>(&Header), sizeof(Header)))
		{
			HERMES_LOG_WARNING(L"Failed to read image header from asset %s", Name.c_str());
			return nullptr;
		}

		uint32 BytesPerPixel = NumberOfChannelInImageFormat(Header.Format) * Header.BytesPerChannel;
		size_t TotalBytes = static_cast<size_t>(Header.Width) * Header.Height * BytesPerPixel;
		std::vector<uint8> ImageData(TotalBytes, 0x00);
		if (!File.Read(ImageData.data(), TotalBytes))
		{
			HERMES_LOG_WARNING(L"Failed to read image data from asset %s", Name.c_str());
			return nullptr;
		}

		auto Result = std::shared_ptr<ImageAsset>(new ImageAsset(Name, Vec2ui { Header.Width, Header.Height },
		                                                         Header.Format, Header.BytesPerChannel,
		                                                         ImageData.data()));

		return Result;
	}

	std::shared_ptr<Asset> AssetLoader::LoadMesh(IPlatformFile& File, const AssetHeader& AssetHeader, const String& Name)
	{
		(void)AssetHeader;
		PACKED_STRUCT_BEGIN
		struct MeshHeader
		{
			uint32 VertexCount;
			uint32 IndexCount;
		};
		PACKED_STRUCT_END

		MeshHeader Header;
		if (!File.Read(reinterpret_cast<uint8*>(&Header), sizeof(Header)))
		{
			HERMES_LOG_WARNING(L"Failed to read mesh header from asset %s", Name.c_str());
			return nullptr;
		}

		std::vector<Vertex> Vertices(Header.VertexCount);
		std::vector<uint32> Indices(Header.IndexCount);
		if (!File.Read(reinterpret_cast<uint8*>(Vertices.data()), Header.VertexCount * sizeof(Vertex)) ||
			!File.Read(reinterpret_cast<uint8*>(Indices.data()), Header.IndexCount * sizeof(uint32)))
		{
			HERMES_LOG_WARNING(L"Failed to read mesh data from asset %s", Name.c_str());
			return nullptr;
		}

		return std::shared_ptr<MeshAsset>(new MeshAsset(Name, Vertices, Indices));
	}
}
