#include "MeshBuffer.h"

#include "RenderingEngine/GPUInteractionUtilities.h"
#include "RenderingEngine/Renderer.h"
#include "Vulkan/Device.h"

namespace Hermes
{
	MeshBuffer::MeshBuffer(const MeshAsset& Asset)
		: DataUploadFinished(false)
		, IndexCount(0)
	{
		auto& Device = Renderer::Get().GetActiveDevice();

		VertexBuffer = Device.CreateBuffer(Asset.GetRequiredVertexBufferSize(),
		                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		IndexBuffer = Device.CreateBuffer(Asset.GetRequiredIndexBufferSize(),
		                                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

		GPUInteractionUtilities::UploadDataToGPUBuffer(Asset.GetRawVertexData(), Asset.GetRequiredVertexBufferSize(), 0, *VertexBuffer);
		GPUInteractionUtilities::UploadDataToGPUBuffer(Asset.GetRawIndexData(), Asset.GetRequiredIndexBufferSize(), 0, *IndexBuffer);

		IndexCount = Asset.GetIndexCount();
		DataUploadFinished = true;
	}

	std::shared_ptr<MeshBuffer> MeshBuffer::CreateFromAsset(const MeshAsset& Asset)
	{
		return std::shared_ptr<MeshBuffer>(new MeshBuffer(Asset));
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
