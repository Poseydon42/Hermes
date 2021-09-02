#include "VulkanCommandBuffer.h"

#include "RenderInterface/Vulkan/VulkanPipeline.h"
#include "RenderInterface/Vulkan/VulkanDevice.h"
#include "RenderInterface/Vulkan/VulkanRenderPass.h"
#include "RenderInterface/Vulkan/VulkanRenderTarget.h"
#include "RenderInterface/Vulkan/VulkanBuffer.h"

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

		void VulkanCommandBuffer::CopyBuffer(const std::shared_ptr<RenderInterface::Buffer>& Source,
		                                     const std::shared_ptr<RenderInterface::Buffer>& Destination,
		                                     std::vector<RenderInterface::BufferCopyRegion> CopyRegions)
		{
			const auto* VulkanSourceBuffer = static_cast<VulkanBuffer*>(Source.get());
			const auto* VulkanDestinationBuffer = static_cast<VulkanBuffer*>(Destination.get());
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
			vkCmdCopyBuffer(Buffer, VulkanSourceBuffer->GetBuffer(), VulkanDestinationBuffer->GetBuffer(), (uint32)VulkanCopyRegions.size(), VulkanCopyRegions.data());
		}
	}
}
