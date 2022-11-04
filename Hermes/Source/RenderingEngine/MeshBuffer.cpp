#include "MeshBuffer.h"

#include "RenderingEngine/GPUInteractionUtilities.h"
#include "RenderingEngine/Renderer.h"
#include "Vulkan/Device.h"

namespace Hermes
{
	MeshBuffer::MeshBuffer(std::weak_ptr<MeshAsset> InAsset)
		: DataUploadFinished(false)
		, IndexCount(0)
		, Asset(std::move(InAsset))
	{
		auto LockedAsset = Asset.lock();
		if (!LockedAsset)
			return;

		auto& Device = Renderer::Get().GetActiveDevice();

		VertexBuffer = Device.CreateBuffer(LockedAsset->GetRequiredVertexBufferSize(),
		                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		IndexBuffer = Device.CreateBuffer(LockedAsset->GetRequiredIndexBufferSize(),
		                                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

		GPUInteractionUtilities::UploadDataToGPUBuffer(LockedAsset->GetRawVertexData(), LockedAsset->GetRequiredVertexBufferSize(), 0, *VertexBuffer);
		GPUInteractionUtilities::UploadDataToGPUBuffer(LockedAsset->GetRawIndexData(), LockedAsset->GetRequiredIndexBufferSize(), 0, *IndexBuffer);

		IndexCount = LockedAsset->GetIndexCount();
		DataUploadFinished = true;
	}

	std::shared_ptr<MeshBuffer> MeshBuffer::CreateFromAsset(std::weak_ptr<MeshAsset> InAsset)
	{
		if (InAsset.expired())
			return nullptr;
		return std::shared_ptr<MeshBuffer>(new MeshBuffer(std::move(InAsset)));
	}

	const Vulkan::Buffer& MeshBuffer::GetVertexBuffer() const
	{
		return *VertexBuffer;
	}

	const Vulkan::Buffer& MeshBuffer::GetIndexBuffer() const
	{
		return *IndexBuffer;
	}

	bool MeshBuffer::IsReady() const
	{
		return VertexBuffer && IndexBuffer && DataUploadFinished;
	}

	MeshBuffer::MeshDrawInformation MeshBuffer::GetDrawInformation() const
	{
		HERMES_ASSERT(IsReady());

		MeshDrawInformation Result = {};
		Result.IndexCount = IndexCount;
		Result.IndexOffset = 0;
		Result.VertexOffset = 0;

		return Result;
	}

}
