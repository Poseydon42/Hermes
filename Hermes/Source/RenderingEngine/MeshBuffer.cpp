#include "MeshBuffer.h"

#include "RenderingEngine/GPUInteractionUtilities.h"
#include "RenderingEngine/Renderer.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/Buffer.h"

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

		VertexBuffer = Device.CreateBuffer(LockedAsset->GetRequiredVertexBufferSize(), RenderInterface::BufferUsageType::VertexBuffer | RenderInterface::BufferUsageType::CopyDestination);
		IndexBuffer = Device.CreateBuffer(LockedAsset->GetRequiredIndexBufferSize(), RenderInterface::BufferUsageType::IndexBuffer | RenderInterface::BufferUsageType::CopyDestination);

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

	const RenderInterface::Buffer& MeshBuffer::GetVertexBuffer() const
	{
		return *VertexBuffer;
	}

	const RenderInterface::Buffer& MeshBuffer::GetIndexBuffer() const
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

		MeshDrawInformation Result;
		Result.IndexCount = IndexCount;
		Result.IndexOffset = 0;
		Result.VertexOffset = 0;

		return Result;
	}

}
