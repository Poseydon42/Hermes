#pragma once

#include <span>
#include <vector>

#include "AssetSystem/Asset.h"
#include "Core/Core.h"
#include "Math/BoundingVolume.h"
#include "Math/Math.h"
#include "RenderingEngine/Resource/MeshResource.h"

namespace Hermes
{
	struct Vertex
	{
		Vec3 Position;
		Vec2 TextureCoordinates;
		Vec3 Normal;
		Vec3 Tangent;
	};
	
	class HERMES_API MeshAsset : public Asset
	{
	public:
		struct Primitive
		{
			uint32 IndexBufferOffset;
			uint32 IndexCount;
		};

		virtual bool IsValid() const override;

		virtual const Resource* GetResource() const override;

		const Vertex* GetRawVertexData() const;

		const uint32* GetRawIndexData() const;

		uint32 GetVertexCount() const;

		uint32 GetIndexCount() const;

		std::span<const Primitive> GetPrimitives() const;

		uint32 GetTriangleCount() const;

		size_t GetRequiredVertexBufferSize() const;

		size_t GetRequiredIndexBufferSize() const;

		const SphereBoundingVolume& GetBoundingVolume() const;

	private:
		MeshAsset(const String& Name, std::vector<Vertex> VertexData, std::vector<uint32> IndexData, std::vector<Primitive> InPrimitives);

		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices;

		std::vector<Primitive> Primitives;

		std::unique_ptr<MeshResource> Resource;

		SphereBoundingVolume BoundingVolume;

		float CalculateMeshRadius() const;

		friend class AssetLoader;
	};
}
