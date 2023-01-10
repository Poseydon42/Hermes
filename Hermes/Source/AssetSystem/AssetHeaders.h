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
		uint32 VertexCount;
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
