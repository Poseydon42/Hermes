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
		static std::shared_ptr<MeshBuffer> CreateFromAsset(std::weak_ptr<MeshAsset> InAsset);

		const Vulkan::Buffer& GetVertexBuffer() const;
		const Vulkan::Buffer& GetIndexBuffer() const;

		bool IsReady() const;

		struct MeshDrawInformation
		{
			uint32 IndexCount;
			uint32 IndexOffset;
			uint32 VertexOffset;
		};

		MeshDrawInformation GetDrawInformation() const;

	private:
		explicit MeshBuffer(std::weak_ptr<MeshAsset> InAsset);

		bool DataUploadFinished;
		uint32 IndexCount;
		std::weak_ptr<MeshAsset> Asset;
		std::shared_ptr<Vulkan::Buffer> VertexBuffer, IndexBuffer;
	};
}
