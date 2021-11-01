#include "GPUInteractionUtilities.h"

#include "RenderInterface/GenericRenderInterface/CommandBuffer.h"
#include "RenderInterface/GenericRenderInterface/Fence.h"
#include "Math/Common.h"
#include "RenderingEngine/Renderer.h"

namespace Hermes
{
	std::shared_ptr<RenderInterface::Buffer> GPUInteractionUtilities::StagingBuffer;

	void GPUInteractionUtilities::UploadDataToGPUBuffer(const void* Data, size_t DataSize, size_t TargetOffset, RenderInterface::Buffer& Target)
	{
		auto& Device = Renderer::Get().GetActiveDevice();

		auto& CurrentStagingBuffer = EnsureStagingBuffer();
		auto& TransferQueue = Device.GetQueue(RenderInterface::QueueType::Transfer);
		auto TransferCommandBuffer = TransferQueue.CreateCommandBuffer(true);
		auto TransferFinishedFence = Device.CreateFence(true);
		HERMES_ASSERT(TargetOffset + DataSize <= Target.GetSize())

		for (size_t DataOffset = 0; DataOffset < DataSize; DataOffset += CurrentStagingBuffer.GetSize())
		{
			size_t NumBytesToCopy = Math::Min(DataSize - DataOffset, CurrentStagingBuffer.GetSize());

			auto* MappedBufferData = CurrentStagingBuffer.Map();
			memcpy(MappedBufferData, reinterpret_cast<const uint8*>(Data) + DataOffset, NumBytesToCopy);
			CurrentStagingBuffer.Unmap();

			TransferFinishedFence->Wait(UINT64_MAX);
			TransferFinishedFence->Reset();

			TransferCommandBuffer->BeginRecording();
			RenderInterface::BufferCopyRegion Region;
			Region.NumBytes = NumBytesToCopy;
			Region.SourceOffset = 0;
			Region.DestinationOffset = DataOffset;
			TransferCommandBuffer->CopyBuffer(CurrentStagingBuffer, Target, { Region });
			TransferCommandBuffer->EndRecording();

			TransferQueue.SubmitCommandBuffer(TransferCommandBuffer, TransferFinishedFence);
			TransferFinishedFence->Wait(UINT64_MAX);
		}
	}

	RenderInterface::Buffer& GPUInteractionUtilities::EnsureStagingBuffer()
	{
		if (StagingBuffer)
			return *StagingBuffer;

		static constexpr size_t StagingBufferSize = 8 * 1024 * 1024;

		auto& Device = Renderer::Get().GetActiveDevice();
		StagingBuffer = Device.CreateBuffer(StagingBufferSize, RenderInterface::BufferUsageType::CPUAccessible | RenderInterface::BufferUsageType::CopySource);
		HERMES_ASSERT_LOG(StagingBuffer, L"Failed to create staging buffer with size %ull", StagingBufferSize);
		return *StagingBuffer;
	}
}
