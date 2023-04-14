#include "Mesh.h"

#include "AssetSystem/AssetLoader.h"
#include "RenderingEngine/GPUInteractionUtilities.h"
#include "RenderingEngine/Renderer.h"
#include "Vulkan/Device.h"

namespace Hermes
{
	HERMES_ADD_BINARY_ASSET_LOADER(Mesh, Mesh);

	Mesh::Mesh(String Name, std::span<const Vertex> Vertices, std::span<const uint32> Indices, std::vector<PrimitiveDrawInformation> InPrimitives)
		: Asset(std::move(Name), AssetType::Mesh)
		, Primitives(std::move(InPrimitives))
		, BoundingVolume(CalculateMeshRadius(Vertices))
	{
		auto& Device = Renderer::GetDevice();

		auto VertexBufferSize = Vertices.size() * sizeof(Vertex);
		VertexBuffer = Device.CreateBuffer(VertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

		auto IndexBufferSize = Indices.size() * sizeof(uint32);
		IndexBuffer = Device.CreateBuffer(IndexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

		GPUInteractionUtilities::UploadDataToGPUBuffer(Vertices.data(), VertexBufferSize, 0, *VertexBuffer);
		GPUInteractionUtilities::UploadDataToGPUBuffer(Indices.data(), IndexBufferSize, 0, *IndexBuffer);
	}

	float Mesh::CalculateMeshRadius(std::span<const Vertex> Vertices) const
	{
		float MaxDistanceSquared = 0.0f;
		for (const auto& Vertex : Vertices)
		{
			MaxDistanceSquared = Math::Max(MaxDistanceSquared, Vertex.Position.LengthSq());
		}

		return Math::Sqrt(MaxDistanceSquared);
	}

	AssetHandle<Mesh> Mesh::Create(String Name, std::span<const Vertex> Vertices, std::span<const uint32> Indices, std::vector<PrimitiveDrawInformation> Primitives)
	{
		return AssetHandle<Mesh>(new Mesh(std::move(Name), Vertices, Indices, std::move(Primitives)));
	}

	AssetHandle<Asset> Mesh::Load(String Name, std::span<const uint8> BinaryData)
	{
		const uint8* DataPtr = BinaryData.data();
		const auto* Header = reinterpret_cast<const MeshAssetHeader*>(DataPtr);
		DataPtr += sizeof(*Header);

		std::vector<PrimitiveDrawInformation> Primitives(Header->PrimitiveCount);
		for (auto& Primitive : Primitives)
		{
			const auto* PrimitiveHeader = reinterpret_cast<const MeshPrimitiveHeader*>(DataPtr);
			DataPtr += sizeof(*PrimitiveHeader);

			Primitive.IndexOffset = PrimitiveHeader->IndexBufferOffset;
			Primitive.IndexCount = PrimitiveHeader->IndexCount;
		}

		std::vector<Vertex> Vertices(Header->VertexBufferSize);
		memcpy(Vertices.data(), DataPtr, Header->VertexBufferSize * sizeof(Vertex));
		DataPtr += Header->VertexBufferSize * sizeof(Vertex);

		std::vector<uint32> Indices(Header->IndexBufferSize);
		memcpy(Indices.data(), DataPtr, Header->IndexBufferSize * sizeof(uint32));
		DataPtr += Header->IndexBufferSize * sizeof(uint32);
		
		HERMES_ASSERT(DataPtr == BinaryData.data() + BinaryData.size());

		return Create(std::move(Name), Vertices, Indices, Primitives);
	}

	const Vulkan::Buffer& Mesh::GetVertexBuffer() const
	{
		return *VertexBuffer;
	}

	const Vulkan::Buffer& Mesh::GetIndexBuffer() const
	{
		return *IndexBuffer;
	}

	std::span<const Mesh::PrimitiveDrawInformation> Mesh::GetPrimitives() const
	{
		return Primitives;
	}

	const SphereBoundingVolume& Mesh::GetBoundingVolume() const
	{
		return BoundingVolume;
	}
}
