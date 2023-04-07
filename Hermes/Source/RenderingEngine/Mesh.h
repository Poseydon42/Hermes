﻿#pragma once

#include <memory>
#include <span>
#include <vector>

#include "AssetSystem/AssetHeaders.h"
#include "Core/Core.h"
#include "Math/BoundingVolume.h"
#include "Vulkan/Buffer.h"

namespace Hermes
{
	class HERMES_API Mesh : public Asset
	{
	public:
		struct PrimitiveDrawInformation
		{
			uint32 IndexOffset;
			uint32 IndexCount;
		};

		static std::unique_ptr<Mesh> Create(String Name, AssetHandle Handle, std::span<const Vertex> Vertices, std::span<const uint32> Indices, std::vector<PrimitiveDrawInformation> Primitives);

		static std::unique_ptr<Asset> Load(String Name, AssetHandle Handle, std::span<const uint8> BinaryData);

		const Vulkan::Buffer& GetVertexBuffer() const;
		const Vulkan::Buffer& GetIndexBuffer() const;
		std::span<const PrimitiveDrawInformation> GetPrimitives() const;

		const SphereBoundingVolume& GetBoundingVolume() const;

	private:
		Mesh(String Name, AssetHandle Handle, std::span<const Vertex> Vertices, std::span<const uint32> Indices, std::vector<PrimitiveDrawInformation> InPrimitives);
		
		std::unique_ptr<Vulkan::Buffer> VertexBuffer, IndexBuffer;

		std::vector<PrimitiveDrawInformation> Primitives;

		SphereBoundingVolume BoundingVolume;

		float CalculateMeshRadius(std::span<const Vertex> Vertices) const;
	};
}