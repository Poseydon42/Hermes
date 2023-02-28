#pragma once

#include <memory>

#include "AssetSystem/MeshAsset.h"
#include "Core/Core.h"
#include "Vulkan/Buffer.h"

namespace Hermes
{
	class HERMES_API MeshBuffer
	{
	public:
		static std::shared_ptr<MeshBuffer> CreateFromAsset(const MeshAsset& Asset);

		const Vulkan::Buffer& GetVertexBuffer() const;
		const Vulkan::Buffer& GetIndexBuffer() const;

		bool IsReady() const;

		struct PrimitiveDrawInformation
		{
			uint32 IndexOffset;
			uint32 IndexCount;
		};

		std::span<const PrimitiveDrawInformation> GetPrimitives() const;

	private:
		explicit MeshBuffer(const MeshAsset& Asset);

		bool DataUploadFinished;
		uint32 IndexCount;
		std::shared_ptr<Vulkan::Buffer> VertexBuffer, IndexBuffer;

		std::vector<PrimitiveDrawInformation> Primitives;
	};
}
