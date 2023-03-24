#pragma once

#include "AssetSystem/Asset.h"
#include "Core/Core.h"
#include "Math/Vector.h"
#include "Math/Vector2.h"

namespace Hermes
{
	enum class ImageFormat : uint8
	{
		Undefined = 0x00,
		R = 0x01,
		RA = 0x09,
		RG = 0x03,
		RGBX = 0x07,
		RGBA = 0x0F,
		HDR = 0x10
	};

	PACKED_STRUCT_BEGIN
	struct AssetHeader
	{
		static constexpr uint8 ExpectedSignature[3] = { 'H', 'A', 'C' };

		uint8 Signature[3];
		AssetType Type;
		// TODO : version, dependencies etc.
	};

	struct MeshAssetHeader
	{
		uint32 VertexBufferSize; // Number of elements in the vertex buffer
		uint32 IndexBufferSize; // Number of elements in the index buffer
		uint32 PrimitiveCount;
	};

	struct Vertex
	{
		Vec3 Position;
		Vec2 TextureCoordinates;
		Vec3 Normal;
		Vec3 Tangent;
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
