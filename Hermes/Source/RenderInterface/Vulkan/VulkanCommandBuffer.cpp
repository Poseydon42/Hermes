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
		
		VulkanCommandBuffer::VulkanCommandBuffer(std::shared_ptr<VulkanDevice> InDevice, VkCommandPool InPool, bool IsPrimaryBuffer)
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

		void VulkanCommandBuffer::BeginRenderPass(const std::shared_ptr<RenderInterface::RenderPass>& RenderPass, const std::shared_ptr<RenderInterface::RenderTarget>& RenderTarget, const std::vector<RenderInterface::ClearColor>& ClearColors)
		{
			VkRenderPassBeginInfo BeginInfo = {};
			BeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			BeginInfo.renderPass  = std::reinterpret_pointer_cast<VulkanRenderPass>(RenderPass)->GetRenderPass();
			BeginInfo.framebuffer = std::reinterpret_pointer_cast<VulkanRenderTarget>(RenderTarget)->GetFramebuffer();
			BeginInfo.renderArea.offset = { 0, 0};
			BeginInfo.renderArea.extent =  { RenderTarget->GetSize().X, RenderTarget->GetSize().Y };
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

		void VulkanCommandBuffer::BindPipeline(const std::shared_ptr<RenderInterface::Pipeline>& Pipeline)
		{
			// TODO : compute pipelines
			vkCmdBindPipeline(Buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, std::reinterpret_pointer_cast<VulkanPipeline>(Pipeline)->GetPipeline());
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
			NewBarrier.subresourceRange.baseMipLevel = 0;
			NewBarrier.subresourceRange.levelCount = Image.GetMipLevelsCount();
			NewBarrier.subresourceRange.baseArrayLayer = 0;
			NewBarrier.subresourceRange.layerCount = 1;
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
				VulkanCopyRegion.bufferRowLength = 0; // TODO : user should be able to specify these
				VulkanCopyRegion.bufferImageHeight = 0;
				VulkanCopyRegion.imageExtent.width = DestinationImage.GetSize().X;
				VulkanCopyRegion.imageExtent.height = DestinationImage.GetSize().Y;
				VulkanCopyRegion.imageExtent.depth = 1;
				VulkanCopyRegion.imageOffset.x = 0;
				VulkanCopyRegion.imageOffset.y = 0;
				VulkanCopyRegion.imageOffset.z = 0;
				VulkanCopyRegion.imageSubresource.aspectMask = VkAspectFlagsFromVkFormat(DataFormatToVkFormat(DestinationImage.GetDataFormat()));
				VulkanCopyRegion.imageSubresource.baseArrayLayer = 0;
				VulkanCopyRegion.imageSubresource.layerCount = 1;
				VulkanCopyRegion.imageSubresource.mipLevel = 0;
				++It;
			}
			vkCmdCopyBufferToImage(Buffer, SourceBuffer.GetBuffer(), DestinationImage.GetImage(), Layout, static_cast<uint32>(VulkanCopyRegions.size()), VulkanCopyRegions.data());
		}
	}
}
