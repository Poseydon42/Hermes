#include "MeshResource.h"

#include "AssetSystem/MeshAsset.h"
#include "RenderingEngine/GPUInteractionUtilities.h"
#include "RenderingEngine/Renderer.h"
#include "Vulkan/Device.h"

namespace Hermes
{
	MeshResource::MeshResource(const MeshAsset& Asset)
		: Resource(Asset.GetName(), ResourceType::Mesh)
	{
		auto& Device = Renderer::Get().GetActiveDevice();

		VertexBuffer = Device.CreateBuffer(Asset.GetRequiredVertexBufferSize(),
		                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		IndexBuffer = Device.CreateBuffer(Asset.GetRequiredIndexBufferSize(),
		                                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

		GPUInteractionUtilities::UploadDataToGPUBuffer(Asset.GetRawVertexData(), Asset.GetRequiredVertexBufferSize(), 0, *VertexBuffer);
		GPUInteractionUtilities::UploadDataToGPUBuffer(Asset.GetRawIndexData(), Asset.GetRequiredIndexBufferSize(), 0, *IndexBuffer);

		for (const auto& Primitive : Asset.GetPrimitives())
		{
			Primitives.emplace_back(Primitive.IndexBufferOffset, Primitive.IndexCount);
		}
	}

	std::unique_ptr<MeshResource> MeshResource::CreateFromAsset(const MeshAsset& Asset)
	{
		return std::unique_ptr<MeshResource>(new MeshResource(Asset));
	}

	const Vulkan::Buffer& MeshResource::GetVertexBuffer() const
	{
		return *VertexBuffer;
	}

	const Vulkan::Buffer& MeshResource::GetIndexBuffer() const
	{
		return *IndexBuffer;
	}

	std::span<const MeshResource::PrimitiveDrawInformation> MeshResource::GetPrimitives() const
	{
		return Primitives;
	}
}
