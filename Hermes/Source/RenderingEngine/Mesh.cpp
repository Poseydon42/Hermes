#include "Mesh.h"

#include "RenderingEngine/GPUInteractionUtilities.h"
#include "RenderingEngine/Renderer.h"
#include "Vulkan/Device.h"

namespace Hermes
{
	DEFINE_ASSET_TYPE(Mesh, Mesh);

	Mesh::Mesh(String Name, AssetHandle Handle, std::span<const Vertex> Vertices, std::span<const uint32> Indices, std::vector<PrimitiveDrawInformation> InPrimitives)
		: Asset(std::move(Name), AssetType::Mesh, Handle)
		, Primitives(std::move(InPrimitives))
		, BoundingVolume(CalculateMeshRadius(Vertices))
	{
		auto& Device = Renderer::Get().GetActiveDevice();

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

	std::unique_ptr<Mesh> Mesh::Create(String Name, AssetHandle Handle, std::span<const Vertex> Vertices, std::span<const uint32> Indices, std::vector<PrimitiveDrawInformation> Primitives)
	{
		return std::unique_ptr<Mesh>(new Mesh(std::move(Name), Handle, Vertices, Indices, std::move(Primitives)));
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
