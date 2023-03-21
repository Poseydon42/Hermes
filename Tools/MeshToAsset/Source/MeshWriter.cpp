#include "MeshWriter.h"

#include <algorithm>
#include <iostream>

#include "AssetSystem/AssetLoader.h"
#include "Mesh.h"
#include "Platform/GenericPlatform/PlatformFile.h"

namespace Hermes::Tools
{
	bool MeshWriter::Write(StringView FileName, const Mesh& Mesh)
	{
		auto File = PlatformFilesystem::OpenFile(FileName, IPlatformFile::FileAccessMode::Write, IPlatformFile::FileOpenMode::Create);
		if (!File)
		{
			std::cerr << "Cannot open output file " << FileName << std::endl;
			return false;
		}

		// At this point the mesh must be triangulated, so the number of indices will be divisible by 4 (because each face is separated
		// by index -1), so we can compute the actual index count using this formula
		std::vector<uint32> FilteredIndices(Mesh.GetIndices().size() / 4 * 3);
		std::ranges::copy_if(Mesh.GetIndices(), FilteredIndices.begin(), [](uint32 Index) { return Index != static_cast<uint32>(-1); });

		// This is also the reason why we need to recompute the primitive data
		auto Primitives = Mesh.GetPrimitives();
		for (auto& Primitive : Primitives)
		{
			Primitive.IndexCount = Primitive.IndexCount / 4 * 3;
			Primitive.IndexBufferOffset = Primitive.IndexBufferOffset / 4 * 3;
		}

		AssetHeader AssetHeader = { .Type = AssetType::Mesh };
		memcpy(AssetHeader.Signature, AssetHeader::ExpectedSignature, sizeof(AssetHeader.Signature));
		MeshAssetHeader MeshHeader = {};
		MeshHeader.VertexBufferSize = static_cast<uint32>(Mesh.GetVertices().size());
		MeshHeader.IndexBufferSize = static_cast<uint32>(FilteredIndices.size());
		MeshHeader.PrimitiveCount = static_cast<uint32>(Primitives.size());

		bool Result =
			File->Write(&AssetHeader, sizeof(AssetHeader)) &&
			File->Write(&MeshHeader, sizeof(MeshHeader)) &&
			File->Write(Primitives.data(), Primitives.size() * sizeof(MeshPrimitiveHeader)) &&
			File->Write(Mesh.GetVertices().data(), Mesh.GetVertices().size() * sizeof(Mesh.GetVertices()[0])) &&
			File->Write(FilteredIndices.data(), FilteredIndices.size() * sizeof(FilteredIndices[0]));

		if (!Result)
		{
			std::cerr << "Failed to write to file " << FileName << std::endl;
		}
		return Result;
	}
}
