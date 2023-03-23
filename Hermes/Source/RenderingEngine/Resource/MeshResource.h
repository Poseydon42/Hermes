#pragma once

#include <memory>
#include <span>

#include "Core/Core.h"
#include "Math/BoundingVolume.h"
#include "RenderingEngine/Resource/Resource.h"
#include "Vulkan/Buffer.h"

namespace Hermes
{
	class MeshAsset;
	struct Vertex;

	class HERMES_API MeshResource : public Resource
	{
	public:
		static std::unique_ptr<MeshResource> CreateFromAsset(const MeshAsset& Asset);

		struct PrimitiveDrawInformation
		{
			uint32 IndexOffset;
			uint32 IndexCount;
		};

		const Vulkan::Buffer& GetVertexBuffer() const;
		const Vulkan::Buffer& GetIndexBuffer() const;
		std::span<const PrimitiveDrawInformation> GetPrimitives() const;

		const SphereBoundingVolume& GetBoundingVolume() const;

	private:
		explicit MeshResource(const MeshAsset& Asset);
		
		std::unique_ptr<Vulkan::Buffer> VertexBuffer, IndexBuffer;

		std::vector<PrimitiveDrawInformation> Primitives;

		SphereBoundingVolume BoundingVolume;

		float CalculateMeshRadius(const Vertex* Vertices, size_t Count) const;
	};
}
