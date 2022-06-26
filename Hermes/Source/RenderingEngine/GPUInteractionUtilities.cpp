#include "GPUInteractionUtilities.h"

#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/Queue.h"
#include "RenderInterface/GenericRenderInterface/Buffer.h"
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

			TransferQueue.SubmitCommandBuffer(*TransferCommandBuffer, TransferFinishedFence.get());
			TransferFinishedFence->Wait(UINT64_MAX);
		}
	}

	void GPUInteractionUtilities::UploadDataToGPUImage(const void* Data, Vec2ui Offset, Vec2ui Dimensions,
	                                                   size_t BytesPerPixel,
	                                                   uint32 MipLevel, RenderInterface::Image& Destination,
	                                                   RenderInterface::ImageLayout CurrentLayout,
	                                                   RenderInterface::ImageLayout LayoutToTransitionTo,
	                                                   std::optional<RenderInterface::CubemapSide> Side)
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
		auto RowsPerSingleTransfer = static_cast<uint32>(StagingBufferSize / BytesPerRow);
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
				Barrier.Side = Side;
				TransferCommandBuffer->InsertImageMemoryBarrier(
					Destination, Barrier,
					RenderInterface::PipelineStage::BottomOfPipe, RenderInterface::PipelineStage::Transfer);
			}

			RenderInterface::BufferToImageCopyRegion Region = {};
			Region.ImageDimensions = { Dimensions.X, RowsToCopy };
			Region.ImageOffset = { Offset.X, Offset.Y + Row };
			Region.MipLevel = MipLevel;
			Region.Side = Side;
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
				Barrier.Side = Side;
				TransferCommandBuffer->InsertImageMemoryBarrier(
					Destination, Barrier,
					RenderInterface::PipelineStage::Transfer, RenderInterface::PipelineStage::TopOfPipe);
			}

			TransferCommandBuffer->EndRecording();

			TransferFinishedFence->Reset();
			TransferQueue.SubmitCommandBuffer(*TransferCommandBuffer, TransferFinishedFence.get());
			TransferFinishedFence->Wait(UINT64_MAX);
		}
	}

	void GPUInteractionUtilities::GenerateMipMaps(RenderInterface::Image& Image,
	                                              RenderInterface::ImageLayout CurrentLayout,
	                                              RenderInterface::ImageLayout LayoutToTransitionTo,
	                                              std::optional<RenderInterface::CubemapSide> Side)
	{
		auto& Device = Renderer::Get().GetActiveDevice();
		
		auto& RenderQueue = Device.GetQueue(RenderInterface::QueueType::Render);
		auto RenderCommandBuffer = RenderQueue.CreateCommandBuffer(true);

		RenderCommandBuffer->BeginRecording();

		Vec2ui SourceMipLevelDimensions = Image.GetSize();
		Vec2ui DestinationMipLevelDimensions = {};
		for (uint32 CurrentMipLevel = 1; CurrentMipLevel < Image.GetMipLevelsCount(); CurrentMipLevel++)
		{
			RenderInterface::ImageMemoryBarrier SourceMipLevelBarrier = {};
			SourceMipLevelBarrier.OperationsThatHaveToEndBefore = RenderInterface::AccessType::MemoryWrite;
			SourceMipLevelBarrier.OperationsThatCanStartAfter = RenderInterface::AccessType::TransferRead;
			// If the mip level that we're going to blit from is 0, then its layout
			// wasn't changed in this function, in other cases layout is Render destination
			// optimal because we've performed blits into it in previous loop iterations
			SourceMipLevelBarrier.OldLayout = CurrentMipLevel == 1 ? CurrentLayout : RenderInterface::ImageLayout::TransferDestinationOptimal;
			SourceMipLevelBarrier.NewLayout = RenderInterface::ImageLayout::TransferSourceOptimal;
			SourceMipLevelBarrier.BaseMipLevel = CurrentMipLevel - 1;
			SourceMipLevelBarrier.MipLevelCount = 1;
			SourceMipLevelBarrier.Side = Side;

			RenderInterface::ImageMemoryBarrier DestinationMipLevelBarrier = {};
			DestinationMipLevelBarrier.OperationsThatHaveToEndBefore = RenderInterface::AccessType::MemoryRead;
			DestinationMipLevelBarrier.OperationsThatCanStartAfter = RenderInterface::AccessType::TransferWrite;
			DestinationMipLevelBarrier.OldLayout = CurrentLayout;
			DestinationMipLevelBarrier.NewLayout = RenderInterface::ImageLayout::TransferDestinationOptimal;
			DestinationMipLevelBarrier.BaseMipLevel = CurrentMipLevel;
			DestinationMipLevelBarrier.MipLevelCount = 1;
			DestinationMipLevelBarrier.Side = Side;

			RenderCommandBuffer->InsertImageMemoryBarrier(
				Image, SourceMipLevelBarrier,
				RenderInterface::PipelineStage::BottomOfPipe, RenderInterface::PipelineStage::Transfer);
			RenderCommandBuffer->InsertImageMemoryBarrier(
				Image, DestinationMipLevelBarrier,
				RenderInterface::PipelineStage::BottomOfPipe, RenderInterface::PipelineStage::Transfer);

			DestinationMipLevelDimensions.X = Math::Max(SourceMipLevelDimensions.X / 2, static_cast<uint32>(1));
			DestinationMipLevelDimensions.Y = Math::Max(SourceMipLevelDimensions.Y / 2, static_cast<uint32>(1));
			RenderInterface::ImageBlitRegion BlitInfo = {};
			BlitInfo.SourceRegion.RectMin = { 0, 0 };
			BlitInfo.SourceRegion.RectMax = SourceMipLevelDimensions;
			BlitInfo.SourceRegion.MipLevel = CurrentMipLevel - 1;
			BlitInfo.SourceRegion.AspectMask = RenderInterface::ImageAspect::Color; // TODO : fix
			BlitInfo.SourceRegion.Side = Side;
			BlitInfo.DestinationRegion.RectMin = { 0, 0 };
			BlitInfo.DestinationRegion.RectMax = DestinationMipLevelDimensions;
			BlitInfo.DestinationRegion.MipLevel = CurrentMipLevel;
			BlitInfo.DestinationRegion.AspectMask = RenderInterface::ImageAspect::Color;
			BlitInfo.DestinationRegion.Side = Side;
			RenderCommandBuffer->BlitImage(
				Image, RenderInterface::ImageLayout::TransferSourceOptimal,
				Image, RenderInterface::ImageLayout::TransferDestinationOptimal,
				{ BlitInfo }, RenderInterface::FilteringMode::Linear);

			SourceMipLevelDimensions = DestinationMipLevelDimensions;
		}

		RenderInterface::ImageMemoryBarrier FinalBarrier = {};
		FinalBarrier.OperationsThatHaveToEndBefore = RenderInterface::AccessType::TransferRead | RenderInterface::AccessType::TransferWrite;
		FinalBarrier.OperationsThatCanStartAfter = RenderInterface::AccessType::MemoryRead | RenderInterface::AccessType::MemoryWrite;
		FinalBarrier.OldLayout = RenderInterface::ImageLayout::Undefined;
		FinalBarrier.NewLayout = LayoutToTransitionTo;
		FinalBarrier.BaseMipLevel = 0;
		FinalBarrier.MipLevelCount = Image.GetMipLevelsCount();
		FinalBarrier.Side = Side;
		RenderCommandBuffer->InsertImageMemoryBarrier(
			Image, FinalBarrier, RenderInterface::PipelineStage::Transfer, RenderInterface::PipelineStage::TopOfPipe);

		RenderCommandBuffer->EndRecording();
		auto FinishFence = Device.CreateFence();
		Device.GetQueue(RenderInterface::QueueType::Render).SubmitCommandBuffer(*RenderCommandBuffer, FinishFence.get());
		FinishFence->Wait(UINT64_MAX);
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
