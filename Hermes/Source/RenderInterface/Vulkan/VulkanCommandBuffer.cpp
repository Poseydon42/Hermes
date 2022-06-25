#include "VulkanCommandBuffer.h"

#include "RenderInterface/Vulkan/VulkanPipeline.h"
#include "RenderInterface/Vulkan/VulkanDevice.h"
#include "RenderInterface/Vulkan/VulkanRenderPass.h"
#include "RenderInterface/Vulkan/VulkanRenderTarget.h"
#include "RenderInterface/Vulkan/VulkanBuffer.h"
#include "RenderInterface/Vulkan/VulkanCommonTypes.h"
#include "RenderInterface/Vulkan/VulkanDescriptor.h"
#include "RenderInterface/Vulkan/VulkanImage.h"
#include "RenderInterface/Vulkan/VulkanQueue.h"

namespace Hermes
{
	namespace Vulkan
	{
		inline VkIndexType IndexSizeToVkIndexType(RenderInterface::IndexSize Size)
		{
			switch (Size)
			{
			case RenderInterface::IndexSize::Uint16:
				return VK_INDEX_TYPE_UINT16;
			case RenderInterface::IndexSize::Uint32:
				return VK_INDEX_TYPE_UINT32;
			default:
				HERMES_ASSERT(false);
				return (VkIndexType)0;
			}
		}
		
		VulkanCommandBuffer::VulkanCommandBuffer(std::shared_ptr<const VulkanDevice> InDevice, VkCommandPool InPool, bool IsPrimaryBuffer)
			: Buffer(VK_NULL_HANDLE)
			, Pool(InPool)
			, Device(std::move(InDevice))
		{
			VkCommandBufferAllocateInfo AllocateInfo = {};
			AllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			AllocateInfo.commandPool = Pool;
			AllocateInfo.commandBufferCount = 1; // TODO : add a way to allocate multiple buffers at one time
			if (IsPrimaryBuffer)
				AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			else
				AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			VK_CHECK_RESULT(vkAllocateCommandBuffers(Device->GetDevice(), &AllocateInfo, &Buffer));
		}

		VulkanCommandBuffer::~VulkanCommandBuffer()
		{
			vkFreeCommandBuffers(Device->GetDevice(), Pool, 1, &Buffer);
		}

		VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& Other)
		{
			*this = std::move(Other);
		}

		VulkanCommandBuffer& VulkanCommandBuffer::operator=(VulkanCommandBuffer&& Other)
		{
			std::swap(Buffer, Other.Buffer);
			std::swap(Device, Other.Device);
			std::swap(Pool, Other.Pool);
			return *this;
		}

		void VulkanCommandBuffer::BeginRecording()
		{
			VkCommandBufferBeginInfo BeginInfo = {};
			BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			VK_CHECK_RESULT(vkBeginCommandBuffer(Buffer, &BeginInfo));
		}

		void VulkanCommandBuffer::EndRecording()
		{
			VK_CHECK_RESULT(vkEndCommandBuffer(Buffer));
		}

		void VulkanCommandBuffer::BeginRenderPass(const RenderInterface::RenderPass& RenderPass, const RenderInterface::RenderTarget& RenderTarget, const std::vector<RenderInterface::ClearColor>& ClearColors)
		{
			VkRenderPassBeginInfo BeginInfo = {};
			BeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			BeginInfo.renderPass  = static_cast<const VulkanRenderPass&>(RenderPass).GetRenderPass();
			BeginInfo.framebuffer = static_cast<const VulkanRenderTarget&>(RenderTarget).GetFramebuffer();
			BeginInfo.renderArea.offset = { 0, 0};
			BeginInfo.renderArea.extent =  { RenderTarget.GetSize().X, RenderTarget.GetSize().Y };
			BeginInfo.clearValueCount = (uint32)ClearColors.size();
			// TODO : this is a dirty hack that assumes that VkClearColor data layout exactly matches ClearColor
			//        data layout. We should fix it one day to be 100% sure in its compatibility
			BeginInfo.pClearValues = (const VkClearValue*)ClearColors.data();
			
			vkCmdBeginRenderPass(Buffer, &BeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		}

		void VulkanCommandBuffer::EndRenderPass()
		{
			vkCmdEndRenderPass(Buffer);
		}

		void VulkanCommandBuffer::BindPipeline(const RenderInterface::Pipeline& Pipeline)
		{
			// TODO : compute pipelines
			vkCmdBindPipeline(Buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, reinterpret_cast<const VulkanPipeline&>(Pipeline).GetPipeline());
		}

		void VulkanCommandBuffer::Draw(uint32 VertexCount, uint32 InstanceCount, uint32 VertexOffset, uint32 InstanceOffset)
		{
			vkCmdDraw(Buffer, VertexCount, InstanceCount, VertexOffset, InstanceOffset);
		}

		void VulkanCommandBuffer::DrawIndexed(uint32 IndexCount, uint32 InstanceCount, uint32 IndexOffset, uint32 VertexOffset, uint32 InstanceOffset)
		{
			vkCmdDrawIndexed(Buffer, IndexCount, InstanceCount, IndexOffset, VertexOffset, InstanceOffset);
		}

		void VulkanCommandBuffer::BindVertexBuffer(const RenderInterface::Buffer& InBuffer)
		{
			VkBuffer TmpBuffer = reinterpret_cast<const VulkanBuffer&>(InBuffer).GetBuffer();
			VkDeviceSize Offset = 0;
			vkCmdBindVertexBuffers(Buffer, 0, 1, &TmpBuffer, &Offset);
		}

		void VulkanCommandBuffer::BindIndexBuffer(const RenderInterface::Buffer& InBuffer, RenderInterface::IndexSize Size)
		{
			VkBuffer TmpBuffer = reinterpret_cast<const VulkanBuffer&>(InBuffer).GetBuffer();
			VkDeviceSize Offset = 0;
			vkCmdBindIndexBuffer(Buffer, TmpBuffer, Offset, IndexSizeToVkIndexType(Size));
		}

		void VulkanCommandBuffer::BindDescriptorSet(const RenderInterface::DescriptorSet& Set, const RenderInterface::Pipeline& Pipeline, uint32 BindingIndex)
		{
			const VkDescriptorSet DescriptorSet = reinterpret_cast<const VulkanDescriptorSet&>(Set).GetDescriptorSet();
			vkCmdBindDescriptorSets(Buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, reinterpret_cast<const VulkanPipeline&>(Pipeline).GetPipelineLayout(), BindingIndex, 1, &DescriptorSet, 0, nullptr);
		}

		void VulkanCommandBuffer::UploadPushConstants(const RenderInterface::Pipeline& Pipeline, RenderInterface::ShaderType ShadersThatUse, const void* Data, uint32 Size, uint32 Offset)
		{
			vkCmdPushConstants(
				Buffer,
				static_cast<const VulkanPipeline&>(Pipeline).GetPipelineLayout(),
				ShaderTypeToVkShaderStage(ShadersThatUse),
				Offset, Size, Data);
		}

		void VulkanCommandBuffer::InsertBufferMemoryBarrier(const RenderInterface::Buffer& InBuffer, const RenderInterface::BufferMemoryBarrier& Barrier, RenderInterface::PipelineStage SourceStage, RenderInterface::PipelineStage DestinationStage)
		{
			VkBufferMemoryBarrier NewBarrier = {};
			NewBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			if (Barrier.OldOwnerQueue.has_value())
				NewBarrier.srcQueueFamilyIndex = static_cast<const VulkanQueue*>(Barrier.OldOwnerQueue.value())->GetQueueFamilyIndex();
			else
				NewBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			if (Barrier.NewOwnerQueue.has_value())
				NewBarrier.dstQueueFamilyIndex = static_cast<const VulkanQueue*>(Barrier.NewOwnerQueue.value())->GetQueueFamilyIndex();
			else
				NewBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			NewBarrier.srcAccessMask = AccessTypeToVkAccessFlags(Barrier.OperationsThatHaveToEndBefore);
			NewBarrier.dstAccessMask = AccessTypeToVkAccessFlags(Barrier.OperationsThatCanStartAfter);
			NewBarrier.buffer = static_cast<const VulkanBuffer&>(InBuffer).GetBuffer();
			NewBarrier.size = Barrier.NumBytes;
			NewBarrier.offset = Barrier.Offset;

			vkCmdPipelineBarrier(Buffer, PipelineStageToVkPipelineStageFlags(SourceStage), PipelineStageToVkPipelineStageFlags(DestinationStage), 0, 0, nullptr, 1, &NewBarrier, 0, nullptr);
		}

		void VulkanCommandBuffer::InsertImageMemoryBarrier(const RenderInterface::Image& Image, const RenderInterface::ImageMemoryBarrier& Barrier, RenderInterface::PipelineStage SourceStage, RenderInterface::PipelineStage DestinationStage)
		{
			VkImageMemoryBarrier NewBarrier = {};
			NewBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			NewBarrier.oldLayout = ImageLayoutToVkImageLayout(Barrier.OldLayout);
			NewBarrier.newLayout = ImageLayoutToVkImageLayout(Barrier.NewLayout);
			if (Barrier.OldOwnerQueue.has_value())
				NewBarrier.srcQueueFamilyIndex = static_cast<const VulkanQueue*>(Barrier.OldOwnerQueue.value())->GetQueueFamilyIndex();
			else
				NewBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			if (Barrier.NewOwnerQueue.has_value())
				NewBarrier.dstQueueFamilyIndex = static_cast<const VulkanQueue*>(Barrier.NewOwnerQueue.value())->GetQueueFamilyIndex();
			else
				NewBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			NewBarrier.srcAccessMask = AccessTypeToVkAccessFlags(Barrier.OperationsThatHaveToEndBefore);
			NewBarrier.dstAccessMask = AccessTypeToVkAccessFlags(Barrier.OperationsThatCanStartAfter);
			NewBarrier.image = static_cast<const VulkanImage&>(Image).GetImage();
			NewBarrier.subresourceRange.baseMipLevel = Barrier.BaseMipLevel;
			NewBarrier.subresourceRange.levelCount = Barrier.MipLevelCount;
			if (Image.IsCubemap())
			{
				HERMES_ASSERT_LOG(Barrier.Side.has_value(),
				              L"Cubemap side was not specified while trying to insert image memory barrier with cubemap image.");
				auto Side = Barrier.Side.value();
				if (Side == RenderInterface::CubemapSide::All)
				{
					NewBarrier.subresourceRange.baseArrayLayer = 0;
					NewBarrier.subresourceRange.layerCount = 6;
				}
				else
				{
					NewBarrier.subresourceRange.baseArrayLayer = CubemapSideToArrayLayer(Side);
					NewBarrier.subresourceRange.layerCount = 1;
				}
			}
			else
			{
				if (Barrier.Side.has_value())
					HERMES_LOG_WARNING(L"Cubemap side specified for image memory barrier with non-cubemap image will be ignored.");
				NewBarrier.subresourceRange.baseArrayLayer = 0;
				NewBarrier.subresourceRange.layerCount = 1;
			}
			NewBarrier.subresourceRange.aspectMask = VkAspectFlagsFromVkFormat(DataFormatToVkFormat(Image.GetDataFormat()));


			vkCmdPipelineBarrier(Buffer, PipelineStageToVkPipelineStageFlags(SourceStage), PipelineStageToVkPipelineStageFlags(DestinationStage), 0, 0, nullptr, 0, nullptr, 1, &NewBarrier);
		}

		void VulkanCommandBuffer::CopyBuffer(const RenderInterface::Buffer& Source, const RenderInterface::Buffer& Destination, std::vector<RenderInterface::BufferCopyRegion> CopyRegions)
		{
			const auto& VulkanSourceBuffer = static_cast<const VulkanBuffer&>(Source);
			const auto& VulkanDestinationBuffer = static_cast<const VulkanBuffer&>(Destination);
			std::vector<VkBufferCopy> VulkanCopyRegions(CopyRegions.size());
			auto It = VulkanCopyRegions.begin();
			for (const auto& CopyRegion : CopyRegions)
			{
				auto& VulkanCopyRegion = *It;
				VulkanCopyRegion.srcOffset = CopyRegion.SourceOffset;
				VulkanCopyRegion.dstOffset = CopyRegion.DestinationOffset;
				VulkanCopyRegion.size = CopyRegion.NumBytes;
				++It;
			}
			vkCmdCopyBuffer(Buffer, VulkanSourceBuffer.GetBuffer(), VulkanDestinationBuffer.GetBuffer(), static_cast<uint32>(VulkanCopyRegions.size()), VulkanCopyRegions.data());
		}

		void VulkanCommandBuffer::CopyBufferToImage(const RenderInterface::Buffer& Source, const RenderInterface::Image& Destination, RenderInterface::ImageLayout DestinationImageLayout, std::vector<RenderInterface::BufferToImageCopyRegion> CopyRegions)
		{
			const auto& SourceBuffer = static_cast<const VulkanBuffer&>(Source);
			const auto& DestinationImage = static_cast<const VulkanImage&>(Destination);
			VkImageLayout Layout = ImageLayoutToVkImageLayout(DestinationImageLayout);

			std::vector<VkBufferImageCopy> VulkanCopyRegions(CopyRegions.size());
			auto It = VulkanCopyRegions.begin();
			for (const auto& CopyRegion : CopyRegions)
			{
				auto& VulkanCopyRegion = *It;
				VulkanCopyRegion.bufferOffset = CopyRegion.BufferOffset;
				VulkanCopyRegion.bufferRowLength = CopyRegion.RowLengthInBuffer.value_or(0);
				VulkanCopyRegion.bufferImageHeight = 0;
				VulkanCopyRegion.imageExtent.width = CopyRegion.ImageDimensions.X;
				VulkanCopyRegion.imageExtent.height = CopyRegion.ImageDimensions.Y;
				VulkanCopyRegion.imageExtent.depth = 1;
				VulkanCopyRegion.imageOffset.x = static_cast<int32>(CopyRegion.ImageOffset.X);
				VulkanCopyRegion.imageOffset.y = static_cast<int32>(CopyRegion.ImageOffset.Y);
				VulkanCopyRegion.imageOffset.z = 0;
				VulkanCopyRegion.imageSubresource.aspectMask = VkAspectFlagsFromVkFormat(DataFormatToVkFormat(DestinationImage.GetDataFormat()));
				if (DestinationImage.IsCubemap())
				{
					HERMES_ASSERT_LOG(CopyRegion.Side.has_value() && CopyRegion.Side.value() != RenderInterface::CubemapSide::All,
				              L"Cubemap side was not specified or it was CubemapSide::All while trying to copy buffer to image");
					VulkanCopyRegion.imageSubresource.baseArrayLayer = CubemapSideToArrayLayer(CopyRegion.Side.value());
					VulkanCopyRegion.imageSubresource.layerCount = 1;
				}
				else
				{
					if (CopyRegion.Side.has_value())
						HERMES_LOG_WARNING(L"Cubemap side specified for non-cubemap image copy region will be ignored.");
					VulkanCopyRegion.imageSubresource.baseArrayLayer = 0;
					VulkanCopyRegion.imageSubresource.layerCount = 1;
				}
				VulkanCopyRegion.imageSubresource.mipLevel = CopyRegion.MipLevel;
				++It;
			}
			vkCmdCopyBufferToImage(Buffer, SourceBuffer.GetBuffer(), DestinationImage.GetImage(), Layout, static_cast<uint32>(VulkanCopyRegions.size()), VulkanCopyRegions.data());
		}

		void VulkanCommandBuffer::CopyImage(const RenderInterface::Image& Source, RenderInterface::ImageLayout SourceLayout, const RenderInterface::Image& Destination, RenderInterface::ImageLayout DestinationLayout, std::vector<RenderInterface::ImageCopyRegion> CopyRegions)
		{
			const auto& VulkanSourceImage      = static_cast<const VulkanImage&>(Source);
			const auto& VulkanDestinationImage = static_cast<const VulkanImage&>(Destination);

			std::vector<VkImageCopy> Regions;
			Regions.reserve(CopyRegions.size());
			for (const auto& CopyRegion : CopyRegions)
			{
				VkImageCopy NewRegion = {};

				NewRegion.extent.width  = CopyRegion.Dimensions.X;
				NewRegion.extent.height = CopyRegion.Dimensions.Y;
				NewRegion.extent.depth  = 1;

				NewRegion.srcOffset.x = CopyRegion.SourceOffset.X;
				NewRegion.srcOffset.y = CopyRegion.SourceOffset.Y;
				NewRegion.srcOffset.z = 0;
				NewRegion.srcSubresource.aspectMask = VkAspectFlagsFromVkFormat(DataFormatToVkFormat(VulkanSourceImage.GetDataFormat()));
				NewRegion.srcSubresource.mipLevel = CopyRegion.SourceMipLayer;

				NewRegion.dstOffset.x = CopyRegion.DestinationOffset.X;
				NewRegion.dstOffset.y = CopyRegion.DestinationOffset.Y;
				NewRegion.dstOffset.z = 0;
				NewRegion.dstSubresource.aspectMask = VkAspectFlagsFromVkFormat(DataFormatToVkFormat(VulkanDestinationImage.GetDataFormat()));
				NewRegion.dstSubresource.mipLevel = CopyRegion.DestinationMipLayer;

				if (Source.IsCubemap())
				{
					HERMES_ASSERT_LOG(CopyRegion.SourceSide.has_value(), L"Unspecified cubemap side for source image");
					if (CopyRegion.SourceSide.value() == RenderInterface::CubemapSide::All)
					{
						NewRegion.srcSubresource.baseArrayLayer = 0;
						NewRegion.srcSubresource.layerCount = 6;
					}
					else
					{
						NewRegion.srcSubresource.baseArrayLayer =
							CubemapSideToArrayLayer(CopyRegion.SourceSide.value());
						NewRegion.srcSubresource.layerCount = 1;
					}
				}
				else
				{
					if (CopyRegion.SourceSide.has_value())
						HERMES_LOG_WARNING(L"Cubemap side specified for copy from non-cubemap source image will be ignored");
					NewRegion.srcSubresource.baseArrayLayer = 0;
					NewRegion.srcSubresource.layerCount = 1;
				}

				if (Destination.IsCubemap())
				{
					HERMES_ASSERT_LOG(CopyRegion.DestinationSide.has_value(),
					                  L"Unspecified cubemap side for destination image");
					if (CopyRegion.DestinationSide.value() == RenderInterface::CubemapSide::All)
					{
						NewRegion.dstSubresource.baseArrayLayer = 0;
						NewRegion.dstSubresource.layerCount = 6;
					}
					else
					{
						NewRegion.dstSubresource.baseArrayLayer =
							CubemapSideToArrayLayer(CopyRegion.DestinationSide.value());
						NewRegion.dstSubresource.layerCount = 1;
					}
				}
				else
				{
					if (CopyRegion.DestinationSide.has_value())
						HERMES_LOG_WARNING(L"Cubemap side specified for copy from non-cubemap destination image will be ignored");
					NewRegion.dstSubresource.baseArrayLayer = 0;
					NewRegion.dstSubresource.layerCount = 1;
				}

				Regions.push_back(NewRegion);
			}

			vkCmdCopyImage(
				Buffer,
				VulkanSourceImage.GetImage(), ImageLayoutToVkImageLayout(SourceLayout),
				VulkanDestinationImage.GetImage(), ImageLayoutToVkImageLayout(DestinationLayout),
				static_cast<uint32>(Regions.size()), Regions.data());
		}

		void VulkanCommandBuffer::BlitImage(const RenderInterface::Image& Source,
			RenderInterface::ImageLayout SourceLayout, const RenderInterface::Image& Destination,
			RenderInterface::ImageLayout DestinationLayout,
			const std::vector<RenderInterface::ImageBlitRegion>& Regions, RenderInterface::FilteringMode Filter)
		{
			const auto& VulkanSourceImage      = static_cast<const VulkanImage&>(Source);
			const auto& VulkanDestinationImage = static_cast<const VulkanImage&>(Destination);

			std::vector<VkImageBlit> VulkanRegions;
			VulkanRegions.reserve(Regions.size());
			for (const auto& Region : Regions)
			{
				VkImageBlit VulkanRegion = {};

				VulkanRegion.srcOffsets[0].x = static_cast<int32>(Region.SourceRegion.RectMin.X);
				VulkanRegion.srcOffsets[0].y = static_cast<int32>(Region.SourceRegion.RectMin.Y);
				VulkanRegion.srcOffsets[1].x = static_cast<int32>(Region.SourceRegion.RectMax.X);
				VulkanRegion.srcOffsets[1].y = static_cast<int32>(Region.SourceRegion.RectMax.Y);
				VulkanRegion.srcOffsets[1].z = 1;
				VulkanRegion.srcSubresource.aspectMask = ImageAspectToVkImageAspectFlags(Region.SourceRegion.AspectMask);
				VulkanRegion.srcSubresource.mipLevel = Region.SourceRegion.MipLevel;

				VulkanRegion.dstOffsets[0].x = static_cast<int32>(Region.DestinationRegion.RectMin.X);
				VulkanRegion.dstOffsets[0].y = static_cast<int32>(Region.DestinationRegion.RectMin.Y);
				VulkanRegion.dstOffsets[1].x = static_cast<int32>(Region.DestinationRegion.RectMax.X);
				VulkanRegion.dstOffsets[1].y = static_cast<int32>(Region.DestinationRegion.RectMax.Y);
				VulkanRegion.dstOffsets[1].z = 1;
				VulkanRegion.dstSubresource.aspectMask = ImageAspectToVkImageAspectFlags(Region.DestinationRegion.AspectMask);
				VulkanRegion.dstSubresource.mipLevel = Region.DestinationRegion.MipLevel;

				if (Source.IsCubemap())
				{
					HERMES_ASSERT_LOG(Region.SourceRegion.Side.has_value(),
					                  L"Unspecified cubemap side for source image");
					if (Region.SourceRegion.Side.value() == RenderInterface::CubemapSide::All)
					{
						VulkanRegion.srcSubresource.baseArrayLayer = 0;
						VulkanRegion.srcSubresource.layerCount = 6;
					}
					else
					{
						VulkanRegion.srcSubresource.baseArrayLayer =
							CubemapSideToArrayLayer(Region.SourceRegion.Side.value());
						VulkanRegion.srcSubresource.layerCount = 1;
					}
				}
				else
				{
					if (Region.SourceRegion.Side.has_value())
						HERMES_LOG_WARNING(L"Cubemap side specified for copy from non-cubemap source image will be ignored");
					VulkanRegion.srcSubresource.baseArrayLayer = 0;
					VulkanRegion.srcSubresource.layerCount = 1;
				}

				if (Destination.IsCubemap())
				{
					HERMES_ASSERT_LOG(Region.DestinationRegion.Side.has_value(),
					                  L"Unspecified cubemap side for destination image");
					if (Region.DestinationRegion.Side.value() == RenderInterface::CubemapSide::All)
					{
						VulkanRegion.dstSubresource.baseArrayLayer = 0;
						VulkanRegion.dstSubresource.layerCount = 6;
					}
					else
					{
						VulkanRegion.dstSubresource.baseArrayLayer =
							CubemapSideToArrayLayer(Region.DestinationRegion.Side.value());
						VulkanRegion.dstSubresource.layerCount = 1;
					}
				}
				else
				{
					if (Region.DestinationRegion.Side.has_value())
						HERMES_LOG_WARNING(L"Cubemap side specified for copy from non-cubemap destination image will be ignored");
					VulkanRegion.dstSubresource.baseArrayLayer = 0;
					VulkanRegion.dstSubresource.layerCount = 1;
				}

				VulkanRegions.push_back(VulkanRegion);
			}

			vkCmdBlitImage(
				Buffer, VulkanSourceImage.GetImage(), ImageLayoutToVkImageLayout(SourceLayout),
				VulkanDestinationImage.GetImage(), ImageLayoutToVkImageLayout(DestinationLayout),
				static_cast<uint32>(VulkanRegions.size()), VulkanRegions.data(), FilteringModeToVkFilter(Filter));
		}
	}
}
