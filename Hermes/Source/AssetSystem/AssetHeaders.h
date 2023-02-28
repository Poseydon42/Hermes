#pragma once

#include "AssetSystem/Asset.h"
#include "AssetSystem/ImageAsset.h"
#include "Core/Core.h"

namespace Hermes
{
	PACKED_STRUCT_BEGIN
	struct AssetHeader
	{
		AssetType Type;
		// TODO : version, dependencies etc.
	};

	struct MeshAssetHeader
	{
		size_t VertexBufferOffset;
		size_t VertexBufferSize; // Number of elements in the vertex buffer

		size_t IndexBufferOffset;
		size_t IndexBufferSize; // Number of elements in the index buffer

		uint32 PrimitiveCount;
	};

	struct MeshPrimitiveHeader
	{
		uint32 IndexBufferOffset;
		uint32 IndexCount;
	};

	struct ImageAssetHeader
	{
		uint16 Width;
		uint16 Height;
		ImageFormat Format;
		uint8 BytesPerChannel;
		uint8 MipLevelCount;
	};
	PACKED_STRUCT_END
}
