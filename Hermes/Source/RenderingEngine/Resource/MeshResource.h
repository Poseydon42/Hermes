#pragma once

#include <memory>
#include <span>

#include "Core/Core.h"
#include "RenderingEngine/Resource/Resource.h"
#include "Vulkan/Buffer.h"

namespace Hermes
{
	class MeshAsset;

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

	private:
		explicit MeshResource(const MeshAsset& Asset);
		
		std::unique_ptr<Vulkan::Buffer> VertexBuffer, IndexBuffer;

		std::vector<PrimitiveDrawInformation> Primitives;
	};
}
