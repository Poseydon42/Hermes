#include "GPUInteractionUtilities.h"

#include "Logging/Logger.h"
#include "Math/Common.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Device.h"
#include "Vulkan/Fence.h"
#include "Vulkan/Image.h"
#include "Vulkan/Queue.h"

namespace Hermes
{
	std::unique_ptr<Vulkan::Buffer> GPUInteractionUtilities::StagingBuffer;

	void GPUInteractionUtilities::ChangeImageLayout(const Vulkan::Image& Image,
		                                            VkImageLayout CurrentLayout, VkImageLayout DestinationLayout,
		                                            VkAccessFlags SourceAccess, VkAccessFlags DestinationAccess,
		                                            VkPipelineStageFlags SourceStage, VkPipelineStageFlagBits DestinationStage)
	{
		const auto& Queue = Renderer::GetDevice().GetQueue(VK_QUEUE_GRAPHICS_BIT);
		auto CommandBuffer = Queue.CreateCommandBuffer();

		CommandBuffer->BeginRecording();

		VkImageMemoryBarrier Barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = SourceAccess,
			.dstAccessMask = DestinationAccess,
			.oldLayout = CurrentLayout,
			.newLayout = DestinationLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = Image.GetImage(),
			.subresourceRange = Image.GetFullSubresourceRange()
		};
		CommandBuffer->InsertImageMemoryBarrier(Barrier, SourceStage, DestinationStage);

		CommandBuffer->EndRecording();
		auto Fence = Renderer::GetDevice().CreateFence();
		Queue.SubmitCommandBuffer(*CommandBuffer, Fence.get());
		Fence->Wait(UINT64_MAX);
	}

	// TODO : do we need to perform ownership transfer between render and transfer queue and vice-versa?
	void GPUInteractionUtilities::UploadDataToGPUBuffer(const void* Data, size_t DataSize, size_t TargetOffset, const Vulkan::Buffer& Target)
	{
		auto& Device = Renderer::GetDevice();

		auto& CurrentStagingBuffer = EnsureStagingBuffer();
		auto& TransferQueue = Device.GetQueue(VK_QUEUE_TRANSFER_BIT);
		auto TransferCommandBuffer = TransferQueue.CreateCommandBuffer(true);
		auto TransferFinishedFence = Device.CreateFence(true);
		HERMES_ASSERT(TargetOffset + DataSize <= Target.GetSize())

		for (size_t DataOffset = 0; DataOffset < DataSize; DataOffset += CurrentStagingBuffer.GetSize())
		{
			size_t NumBytesToCopy = Math::Min(DataSize - DataOffset, CurrentStagingBuffer.GetSize());

			auto* MappedBufferData = CurrentStagingBuffer.Map();
			memcpy(MappedBufferData, static_cast<const uint8*>(Data) + DataOffset, NumBytesToCopy);
			CurrentStagingBuffer.Unmap();

			TransferFinishedFence->Wait(UINT64_MAX);
			TransferFinishedFence->Reset();

			TransferCommandBuffer->BeginRecording();
			VkBufferCopy Copy;
			Copy.size = NumBytesToCopy;
			Copy.srcOffset = 0;
			Copy.dstOffset = TargetOffset + DataOffset;
			TransferCommandBuffer->CopyBuffer(CurrentStagingBuffer, Target, { &Copy, 1 });
			TransferCommandBuffer->EndRecording();

			TransferQueue.SubmitCommandBuffer(*TransferCommandBuffer, TransferFinishedFence.get());
			TransferFinishedFence->Wait(UINT64_MAX);
		}
	}

	void GPUInteractionUtilities::UploadDataToGPUImage(const void* Data, Vec2ui Offset, Vec2ui Dimensions,
	                                                   size_t BytesPerPixel,
	                                                   uint32 MipLevel, Vulkan::Image& Destination,
	                                                   VkImageLayout CurrentLayout,
	                                                   VkImageLayout LayoutToTransitionTo,
	                                                   std::optional<Vulkan::CubemapSide> Side)
	{
		auto& Device = Renderer::GetDevice();

		auto& CurrentStagingBuffer = EnsureStagingBuffer();
		auto& TransferQueue = Device.GetQueue(VK_QUEUE_TRANSFER_BIT);
		auto TransferCommandBuffer = TransferQueue.CreateCommandBuffer(true);
		auto TransferFinishedFence = Device.CreateFence(true);

		HERMES_ASSERT(Offset.X + Dimensions.X <= Destination.GetDimensions().X);
		HERMES_ASSERT(Offset.Y + Dimensions.Y <= Destination.GetDimensions().Y);

		size_t StagingBufferSize = CurrentStagingBuffer.GetSize();
		size_t BytesPerRow = BytesPerPixel * Dimensions.X;
		auto RowsPerSingleTransfer = static_cast<uint32>(StagingBufferSize / BytesPerRow);
		HERMES_ASSERT_LOG(BytesPerRow <= StagingBufferSize, "Trying to upload too large image to GPU; increase staging buffer size");

		for (uint32 Row = 0; Row < Dimensions.Y; Row += RowsPerSingleTransfer)
		{
			uint32 RowsToCopy = std::min(RowsPerSingleTransfer, Dimensions.Y - Row);

			auto* Memory = CurrentStagingBuffer.Map();
			size_t OffsetIntoSourceData = static_cast<size_t>(Row) * Dimensions.X * BytesPerPixel;
			const void* CurrentDataPointer = static_cast<const uint8*>(Data) + OffsetIntoSourceData;
			memcpy(Memory, CurrentDataPointer, RowsToCopy * BytesPerRow);
			CurrentStagingBuffer.Unmap();

			TransferCommandBuffer->BeginRecording();

			if (CurrentLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && Row == 0)
			{
				VkImageMemoryBarrier Barrier = {};
				Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				Barrier.srcAccessMask = VK_ACCESS_NONE;
				Barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				Barrier.oldLayout = CurrentLayout;
				Barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				Barrier.image = Destination.GetImage();
				Barrier.subresourceRange = Destination.GetFullSubresourceRange();
				if (Side.has_value())
					Barrier.subresourceRange.baseArrayLayer = CubemapSideToArrayLayer(Side.value());
				TransferCommandBuffer->InsertImageMemoryBarrier(Barrier, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				                                                VK_PIPELINE_STAGE_TRANSFER_BIT);
			}

			VkBufferImageCopy Copy = {};
			Copy.imageExtent.width = Dimensions.X;
			Copy.imageExtent.height = RowsToCopy;
			Copy.imageExtent.depth = 1;
			Copy.imageOffset.x = static_cast<int32>(Offset.X);
			Copy.imageOffset.y = static_cast<int32>(Offset.Y + Row);
			Copy.imageOffset.z = 0;
			Copy.imageSubresource.mipLevel = MipLevel;
			Copy.imageSubresource.layerCount = Destination.IsCubemap() ? 6 : 1;
			Copy.imageSubresource.aspectMask = Destination.GetFullAspectMask();
			if (Side.has_value())
				Copy.imageSubresource.baseArrayLayer = CubemapSideToArrayLayer(Side.value());
			TransferCommandBuffer->CopyBufferToImage(CurrentStagingBuffer, Destination,
			                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { &Copy, 1 });

			if (Row + RowsPerSingleTransfer >= Dimensions.Y)
			{
				VkImageMemoryBarrier Barrier = {};
				Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				Barrier.dstAccessMask = VK_ACCESS_NONE;
				Barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				Barrier.newLayout = LayoutToTransitionTo;
				Barrier.image = Destination.GetImage();
				Barrier.subresourceRange.baseMipLevel = 0;
				Barrier.subresourceRange.levelCount = Destination.GetMipLevelsCount();
				Barrier.subresourceRange.layerCount = Destination.IsCubemap() ? 6 : 1;
				Barrier.subresourceRange.aspectMask = Vulkan::FullImageAspectMask(Destination.GetDataFormat());
				if (Side.has_value())
					Barrier.subresourceRange.baseArrayLayer = CubemapSideToArrayLayer(Side.value());
				TransferCommandBuffer->InsertImageMemoryBarrier(Barrier, VK_PIPELINE_STAGE_TRANSFER_BIT,
				                                                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
			}

			TransferCommandBuffer->EndRecording();

			TransferFinishedFence->Reset();
			TransferQueue.SubmitCommandBuffer(*TransferCommandBuffer, TransferFinishedFence.get());
			TransferFinishedFence->Wait(UINT64_MAX);
		}
	}

	void GPUInteractionUtilities::GenerateMipMaps(const Vulkan::Image& Image,
	                                              VkImageLayout CurrentLayout,
	                                              VkImageLayout LayoutToTransitionTo,
	                                              std::optional<Vulkan::CubemapSide> Side)
	{
		auto& Device = Renderer::GetDevice();
		
		auto& Queue = Device.GetQueue(VK_QUEUE_GRAPHICS_BIT);
		auto CommandBuffer = Queue.CreateCommandBuffer(true);

		CommandBuffer->BeginRecording();

		auto SourceMipLevelDimensions = Vec2i(Image.GetDimensions());
		Vec2i DestinationMipLevelDimensions = {};
		for (uint32 CurrentMipLevel = 1; CurrentMipLevel < Image.GetMipLevelsCount(); CurrentMipLevel++)
		{
			VkImageMemoryBarrier PerLevelMemoryBarriers[2] = {};

			// Source mip level barrier
			PerLevelMemoryBarriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			PerLevelMemoryBarriers[0].image = Image.GetImage();
			PerLevelMemoryBarriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			PerLevelMemoryBarriers[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			// If the mip level that we're going to blit from is 0, then its layout
			// wasn't changed in this function, in otherwise layout is in transfer destination
			// optimal layout because we've performed blits into it in previous loop iterations
			PerLevelMemoryBarriers[0].oldLayout = (CurrentMipLevel == 1
				                                   ? CurrentLayout
				                                   : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			PerLevelMemoryBarriers[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			PerLevelMemoryBarriers[0].subresourceRange.aspectMask = Image.GetFullAspectMask();
			PerLevelMemoryBarriers[0].subresourceRange.baseArrayLayer = Side.has_value()
				                                                            ? CubemapSideToArrayLayer(Side.value())
				                                                            : 0;
			PerLevelMemoryBarriers[0].subresourceRange.layerCount = 1;
			PerLevelMemoryBarriers[0].subresourceRange.baseMipLevel = CurrentMipLevel - 1;
			PerLevelMemoryBarriers[0].subresourceRange.levelCount = 1;

			// Destination mip level barrier
			PerLevelMemoryBarriers[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			PerLevelMemoryBarriers[1].image = Image.GetImage();
			PerLevelMemoryBarriers[1].srcAccessMask = VK_ACCESS_NONE;
			PerLevelMemoryBarriers[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			PerLevelMemoryBarriers[1].oldLayout = CurrentLayout;
			PerLevelMemoryBarriers[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			PerLevelMemoryBarriers[1].subresourceRange.aspectMask = Image.GetFullAspectMask();
			PerLevelMemoryBarriers[1].subresourceRange.baseMipLevel = CurrentMipLevel;
			PerLevelMemoryBarriers[1].subresourceRange.levelCount = 1;
			PerLevelMemoryBarriers[1].subresourceRange.layerCount = 1;
			if (Side.has_value())
				PerLevelMemoryBarriers[1].subresourceRange.baseArrayLayer = CubemapSideToArrayLayer(Side.value());

			CommandBuffer->InsertImageMemoryBarriers(PerLevelMemoryBarriers, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

			DestinationMipLevelDimensions.X = Math::Max(SourceMipLevelDimensions.X / 2, 1);
			DestinationMipLevelDimensions.Y = Math::Max(SourceMipLevelDimensions.Y / 2, 1);

			VkImageBlit BlitInfo = {};
			BlitInfo.srcOffsets[0] = { 0, 0, 0 };
			BlitInfo.srcOffsets[1] = { SourceMipLevelDimensions.X, SourceMipLevelDimensions.Y, 1 };
			BlitInfo.srcSubresource.mipLevel = CurrentMipLevel - 1;
			BlitInfo.srcSubresource.aspectMask = Image.GetFullAspectMask();
			BlitInfo.srcSubresource.layerCount = 1;
			if (Side.has_value())
				BlitInfo.srcSubresource.baseArrayLayer = CubemapSideToArrayLayer(Side.value());

			BlitInfo.dstOffsets[0] = { 0, 0, 0 };
			BlitInfo.dstOffsets[1] = { DestinationMipLevelDimensions.X, DestinationMipLevelDimensions.Y, 1 };
			BlitInfo.dstSubresource.mipLevel = CurrentMipLevel;
			BlitInfo.dstSubresource.aspectMask = Image.GetFullAspectMask();
			BlitInfo.dstSubresource.layerCount = 1;
			if (Side.has_value())
				BlitInfo.dstSubresource.baseArrayLayer = CubemapSideToArrayLayer(Side.value());
			CommandBuffer->BlitImage(Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, Image,
			                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { &BlitInfo, 1 }, VK_FILTER_LINEAR);

			SourceMipLevelDimensions = DestinationMipLevelDimensions;
		}

		VkImageMemoryBarrier FinalBarrier = {};
		FinalBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		FinalBarrier.image = Image.GetImage();
		FinalBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		FinalBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
		FinalBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		FinalBarrier.newLayout = LayoutToTransitionTo;
		FinalBarrier.subresourceRange.baseMipLevel = 0;
		// NOTE: because the last mip level will be in VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		FinalBarrier.subresourceRange.levelCount = Image.GetMipLevelsCount() - 1;
		FinalBarrier.subresourceRange.aspectMask = Image.GetFullAspectMask();
		FinalBarrier.subresourceRange.layerCount = 1;
		if (Side.has_value())
			FinalBarrier.subresourceRange.baseArrayLayer = CubemapSideToArrayLayer(Side.value());
		CommandBuffer->InsertImageMemoryBarrier(FinalBarrier, VK_PIPELINE_STAGE_TRANSFER_BIT,
		                                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

		// NOTE: insert the final barrier once again, but this time for the last mip level
		FinalBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		FinalBarrier.subresourceRange.baseMipLevel = Image.GetMipLevelsCount() - 1;
		FinalBarrier.subresourceRange.levelCount = 1;
		CommandBuffer->InsertImageMemoryBarrier(FinalBarrier, VK_PIPELINE_STAGE_TRANSFER_BIT,
		                                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

		CommandBuffer->EndRecording();
		auto FinishFence = Device.CreateFence();
		Queue.SubmitCommandBuffer(*CommandBuffer, FinishFence.get());
		FinishFence->Wait(UINT64_MAX);
	}

	void GPUInteractionUtilities::ClearImage(const Vulkan::Image& Image, Vec4 Color, std::span<const VkImageSubresourceRange> Ranges, VkImageLayout CurrentLayout, VkImageLayout LayoutToTransitionTo)
	{
		const auto& Queue = Renderer::GetDevice().GetQueue(VK_QUEUE_GRAPHICS_BIT);
		auto CommandBuffer = Queue.CreateCommandBuffer();

		CommandBuffer->BeginRecording();

		VkImageMemoryBarrier BeforeClearBarrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.oldLayout = CurrentLayout,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = Image.GetImage(),
			.subresourceRange = Image.GetFullSubresourceRange()
		};
		CommandBuffer->InsertImageMemoryBarrier(BeforeClearBarrier, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		VkClearColorValue ClearColor;
		ClearColor.float32[0] = Color.X;
		ClearColor.float32[1] = Color.Y;
		ClearColor.float32[2] = Color.Z;
		ClearColor.float32[3] =	Color.W;
		CommandBuffer->ClearColorImage(Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, ClearColor, Ranges);

		VkImageMemoryBarrier AfterClearBarrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = LayoutToTransitionTo,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = Image.GetImage(),
			.subresourceRange = Image.GetFullSubresourceRange()
		};
		CommandBuffer->InsertImageMemoryBarrier(AfterClearBarrier, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

		CommandBuffer->EndRecording();
		auto Fence = Renderer::GetDevice().CreateFence();
		Queue.SubmitCommandBuffer(*CommandBuffer, Fence.get());
		Fence->Wait(UINT64_MAX);
	}

	Vulkan::Buffer& GPUInteractionUtilities::EnsureStagingBuffer()
	{
		if (StagingBuffer)
			return *StagingBuffer;

		static constexpr size_t StagingBufferSize = 8ull * 1024 * 1024;

		auto& Device = Renderer::GetDevice();
		StagingBuffer = Device.CreateBuffer(StagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
		HERMES_ASSERT_LOG(StagingBuffer, "Failed to create staging buffer with size %ull", StagingBufferSize);
		return *StagingBuffer;
	}
}
