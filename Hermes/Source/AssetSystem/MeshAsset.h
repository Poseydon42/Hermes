#pragma once

#include "AssetSystem/Asset.h"
#include "Core/Core.h"
#include "Math/Math.h"

namespace Hermes
{
	struct Vertex
	{
		Vec3 Position;
		Vec2 TextureCoordinates;
		Vec3 Normal;
		Vec3 Tangent;
	};

	/*
	 * In Hermes' representation, mesh is simply an array of raw vertex and index data for object to be
	 * drawn on the screen. It does not contain any material, hierarchy or animation information.
	 * */
	class HERMES_API MeshAsset : public Asset
	{
	public:
		virtual size_t GetMemorySize() const override;

		virtual bool IsValid() const override;

		const Vertex* GetRawVertexData() const;

		const uint32* GetRawIndexData() const;

		uint32 GetVertexCount() const;

		uint32 GetIndexCount() const;

		uint32 GetTriangleCount() const;

		size_t GetRequiredVertexBufferSize() const;

		size_t GetRequiredIndexBufferSize() const;
	private:
		MeshAsset(const String& Name, std::vector<Vertex> VertexData, std::vector<uint32> IndexData);

		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices;

		friend class AssetLoader;
	};
}
