#include "GPUInteractionUtilities.h"

#include "RenderInterface/GenericRenderInterface/CommandBuffer.h"
#include "RenderInterface/GenericRenderInterface/Fence.h"
#include "Math/Common.h"
#include "RenderingEngine/Renderer.h"
#include "RenderInterface/GenericRenderInterface/Image.h"

namespace Hermes
{
	std::shared_ptr<RenderInterface::Buffer> GPUInteractionUtilities::StagingBuffer;

	// TODO : do we need to perform ownership transfer between render and transfer queue and vice-versa?
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

	void GPUInteractionUtilities::UploadDataToGPUImage(
		const void* Data, Vec2ui Offset, Vec2ui Dimensions, size_t BytesPerPixel,
		uint32 MipLevel, RenderInterface::Image& Destination,
		RenderInterface::ImageLayout CurrentLayout, RenderInterface::ImageLayout LayoutToTransitionTo)
	{
		auto& Device = Renderer::Get().GetActiveDevice();

		auto& CurrentStagingBuffer = EnsureStagingBuffer();
		auto& TransferQueue = Device.GetQueue(RenderInterface::QueueType::Transfer);
		auto TransferCommandBuffer = TransferQueue.CreateCommandBuffer(true);
		auto TransferFinishedFence = Device.CreateFence(true);

		HERMES_ASSERT(Offset.X + Dimensions.X <= Destination.GetSize().X);
		HERMES_ASSERT(Offset.Y + Dimensions.Y <= Destination.GetSize().Y);

		size_t StagingBufferSize = CurrentStagingBuffer.GetSize();
		size_t BytesPerRow = BytesPerPixel * Dimensions.X;
		auto RowsPerSingleTransfer = static_cast<uint32>(StagingBufferSize / BytesPerRow * BytesPerRow);
		HERMES_ASSERT_LOG(BytesPerRow <= StagingBufferSize, L"Trying to upload too large image to GPU; increase staging buffer size");

		for (uint32 Row = 0; Row < Dimensions.Y; Row += RowsPerSingleTransfer)
		{
			uint32 RowsToCopy = std::min(RowsPerSingleTransfer, Dimensions.Y - Row);

			auto* Memory = CurrentStagingBuffer.Map();
			size_t OffsetIntoSourceData = static_cast<size_t>(Row) * Dimensions.X * BytesPerPixel;
			const void* CurrentDataPointer = reinterpret_cast<const uint8*>(Data) + OffsetIntoSourceData;
			memcpy(Memory, CurrentDataPointer, RowsToCopy * BytesPerRow);
			CurrentStagingBuffer.Unmap();

			TransferCommandBuffer->BeginRecording();

			if (CurrentLayout != RenderInterface::ImageLayout::TransferDestinationOptimal && Row == 0)
			{
				RenderInterface::ImageMemoryBarrier Barrier = {};
				Barrier.OperationsThatHaveToEndBefore = RenderInterface::AccessType::None;
				Barrier.OperationsThatCanStartAfter = RenderInterface::AccessType::TransferWrite;
				Barrier.OldLayout = CurrentLayout;
				Barrier.NewLayout = RenderInterface::ImageLayout::TransferDestinationOptimal;
				Barrier.BaseMipLevel = 0;
				Barrier.MipLevelCount = Destination.GetMipLevelsCount();
				TransferCommandBuffer->InsertImageMemoryBarrier(
					Destination, Barrier,
					RenderInterface::PipelineStage::BottomOfPipe, RenderInterface::PipelineStage::Transfer);
			}

			RenderInterface::BufferToImageCopyRegion Region = {};
			Region.ImageDimensions = { Dimensions.X, RowsToCopy };
			Region.ImageOffset = { Offset.X, Offset.Y + Row };
			Region.MipLevel = MipLevel;
			TransferCommandBuffer->CopyBufferToImage(
				CurrentStagingBuffer, Destination, RenderInterface::ImageLayout::TransferDestinationOptimal,
				{ Region });

			if (Row + RowsPerSingleTransfer >= Dimensions.Y)
			{
				RenderInterface::ImageMemoryBarrier Barrier = {};
				Barrier.OperationsThatHaveToEndBefore = RenderInterface::AccessType::TransferWrite;
				Barrier.OperationsThatCanStartAfter = RenderInterface::AccessType::None;
				Barrier.OldLayout = RenderInterface::ImageLayout::TransferDestinationOptimal;
				Barrier.NewLayout = LayoutToTransitionTo;
				Barrier.BaseMipLevel = 0;
				Barrier.MipLevelCount = Destination.GetMipLevelsCount();
				TransferCommandBuffer->InsertImageMemoryBarrier(
					Destination, Barrier,
					RenderInterface::PipelineStage::Transfer, RenderInterface::PipelineStage::TopOfPipe);
			}

			TransferCommandBuffer->EndRecording();

			TransferFinishedFence->Reset();
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
