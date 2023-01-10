#include "MeshWriter.h"

#include <iostream>

#include "AssetSystem/AssetLoader.h"
#include "Platform/GenericPlatform/PlatformFile.h"

namespace Hermes::Tools
{
	bool MeshWriter::Write(StringView FileName, std::span<const Vertex> Vertices, std::span<const uint32> Indices)
	{
		auto File = PlatformFilesystem::OpenFile(FileName, IPlatformFile::FileAccessMode::Write, IPlatformFile::FileOpenMode::Create);
		if (!File || !File->IsValid())
		{
			std::cerr << "Cannot open output file " << FileName << std::endl;
			return false;
		}

		AssetHeader Header = { AssetType::Mesh };
		MeshAssetHeader MeshHeader = { static_cast<uint32>(Vertices.size()), static_cast<uint32>(Indices.size()) };

		bool Result =
			File->Write(&Header, sizeof(Header)) &&
			File->Write(&MeshHeader, sizeof(MeshHeader)) &&
			File->Write(Vertices.data(), Vertices.size_bytes()) &&
			File->Write(Indices.data(), Indices.size_bytes());

		if (!Result)
		{
			std::cerr << "Failed to write to file " << FileName << std::endl;
		}
		return Result;
	}
}
