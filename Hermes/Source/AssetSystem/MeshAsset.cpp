#include "MeshAsset.h"

#include "Logging/Logger.h"

namespace Hermes
{
	DEFINE_ASSET_TYPE(Mesh)

	size_t MeshAsset::GetMemorySize() const
	{
		return Indices.size() * sizeof(Indices[0]) + Vertices.size() * sizeof(Vertices[0]);
	}

	bool MeshAsset::IsValid() const
	{
		return (Indices.size() % 3 == 0 && !Indices.empty() && !Vertices.empty());
	}

	const Vertex* MeshAsset::GetRawVertexData() const
	{
		return Vertices.data();
	}

	const uint32* MeshAsset::GetRawIndexData() const
	{
		return Indices.data();
	}

	uint32 MeshAsset::GetVertexCount() const
	{
		HERMES_ASSERT_LOG(Vertices.size() <= UINT32_MAX, L"Cant load meshes with more than 2^32 - 1 vertices");
		return static_cast<uint32>(Vertices.size());
	}

	uint32 MeshAsset::GetIndexCount() const
	{
		HERMES_ASSERT_LOG(Indices.size() <= UINT32_MAX, L"Cant load meshes with more than 2^32 - 1 indices");
		return static_cast<uint32>(Indices.size());
	}

	uint32 MeshAsset::GetTriangleCount() const
	{
		return GetIndexCount() / 3;
	}

	size_t MeshAsset::GetRequiredVertexBufferSize() const
	{
		return GetVertexCount() * sizeof(Vertices[0]);
	}

	size_t MeshAsset::GetRequiredIndexBufferSize() const
	{
		return GetIndexCount() * sizeof(Indices[0]);
	}

	MeshAsset::MeshAsset(const String& Name, std::vector<Vertex> VertexData, std::vector<uint32> IndexData)
		: Asset(Name, AssetType::Mesh)
		, Vertices(std::move(VertexData))
		, Indices(std::move(IndexData))
	{
	}
}
