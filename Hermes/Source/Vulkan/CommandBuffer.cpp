﻿#include "CommandBuffer.h"

#include "Vulkan/Buffer.h"
#include "Vulkan/ComputePipeline.h"
#include "Vulkan/Descriptor.h"
#include "Vulkan/Image.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/Framebuffer.h"
#include "Vulkan/Queue.h"

namespace Hermes::Vulkan
{
	CommandBuffer::CommandBuffer(std::shared_ptr<Queue::VkQueueHolder> InQueue, bool IsPrimaryBuffer)
		: Queue(std::move(InQueue))
	{
		VkCommandBufferAllocateInfo AllocateInfo = {};
		AllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		AllocateInfo.commandPool = Queue->CommandPool;
		AllocateInfo.commandBufferCount = 1; // TODO : add a way to allocate multiple buffers at one time
		if (IsPrimaryBuffer)
			AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		else
			AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(Queue->Device->Device, &AllocateInfo, &Handle));
	}

	CommandBuffer::~CommandBuffer()
	{
		vkFreeCommandBuffers(Queue->Device->Device, Queue->CommandPool, 1, &Handle);
	}

	void CommandBuffer::BeginRecording()
	{
		VkCommandBufferBeginInfo BeginInfo = {};
		BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VK_CHECK_RESULT(vkBeginCommandBuffer(Handle, &BeginInfo));
	}

	void CommandBuffer::EndRecording()
	{
		VK_CHECK_RESULT(vkEndCommandBuffer(Handle));
	}

	void CommandBuffer::BeginRenderPass(const RenderPass& RenderPass, const Framebuffer& Framebuffer,
	                                    std::span<VkClearValue> ClearColors)
	{
		VkRenderPassBeginInfo BeginInfo = {};
		BeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		BeginInfo.renderPass = RenderPass.GetRenderPass();
		BeginInfo.framebuffer = Framebuffer.GetFramebuffer();
		BeginInfo.renderArea.offset = { 0, 0 };
		BeginInfo.renderArea.extent = { Framebuffer.GetDimensions().X, Framebuffer.GetDimensions().Y };
		BeginInfo.clearValueCount = static_cast<uint32>(ClearColors.size());
		BeginInfo.pClearValues = ClearColors.data();

		vkCmdBeginRenderPass(Handle, &BeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void CommandBuffer::EndRenderPass()
	{
		vkCmdEndRenderPass(Handle);
	}

	void CommandBuffer::BindPipeline(const Pipeline& Pipeline)
	{
		vkCmdBindPipeline(Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.GetPipeline());
	}

	void CommandBuffer::BindPipeline(const ComputePipeline& Pipeline)
	{
		vkCmdBindPipeline(Handle, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline.GetPipeline());
	}

	void CommandBuffer::Draw(uint32 VertexCount, uint32 InstanceCount, uint32 VertexOffset, uint32 InstanceOffset)
	{
		vkCmdDraw(Handle, VertexCount, InstanceCount, VertexOffset, InstanceOffset);
	}

	void CommandBuffer::DrawIndexed(uint32 IndexCount, uint32 InstanceCount, uint32 IndexOffset,
	                                int32 VertexOffset, uint32 InstanceOffset)
	{
		vkCmdDrawIndexed(Handle, IndexCount, InstanceCount, IndexOffset, VertexOffset, InstanceOffset);
	}

	void CommandBuffer::Dispatch(uint32 GroupCountX, uint32 GroupCountY, uint32 GroupCountZ)
	{
		vkCmdDispatch(Handle, GroupCountX, GroupCountY, GroupCountZ);
	}

	void CommandBuffer::BindVertexBuffer(const Buffer& Buffer)
	{
		VkBuffer TmpBuffer = Buffer.GetBuffer();
		VkDeviceSize Offset = 0;
		vkCmdBindVertexBuffers(Handle, 0, 1, &TmpBuffer, &Offset);
	}

	void CommandBuffer::BindIndexBuffer(const Buffer& Buffer, VkIndexType IndexType)
	{
		VkDeviceSize Offset = 0;
		vkCmdBindIndexBuffer(Handle, Buffer.GetBuffer(), Offset, IndexType);
	}

	void CommandBuffer::BindDescriptorSet(const DescriptorSet& Set, const Pipeline& Pipeline, uint32 BindingIndex)
	{
		auto DescriptorSet = Set.GetDescriptorSet();
		vkCmdBindDescriptorSets(Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.GetPipelineLayout(), BindingIndex, 1,
		                        &DescriptorSet, 0, nullptr);
	}

	void CommandBuffer::BindDescriptorSet(const DescriptorSet& Set, const ComputePipeline& Pipeline, uint32 BindingIndex)
	{
		auto DescriptorSet = Set.GetDescriptorSet();
		vkCmdBindDescriptorSets(Handle, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline.GetPipelineLayout(), BindingIndex, 1,
		                        &DescriptorSet, 0, nullptr);
	}

	void CommandBuffer::UploadPushConstants(const Pipeline& Pipeline, VkShaderStageFlags ShadersThatUse,
	                                        const void* Data, uint32 Size, uint32 Offset)
	{
		vkCmdPushConstants(Handle, Pipeline.GetPipelineLayout(), ShadersThatUse, Offset, Size, Data);
	}

	void CommandBuffer::InsertBufferMemoryBarrier(const VkBufferMemoryBarrier& Barrier,
	                                              VkPipelineStageFlags SourceStage,
	                                              VkPipelineStageFlags DestinationStage)
	{
		vkCmdPipelineBarrier(Handle, SourceStage, DestinationStage, 0, 0, nullptr, 1, &Barrier, 0, nullptr);
	}

	void CommandBuffer::InsertBufferMemoryBarriers(std::span<VkBufferMemoryBarrier> Barriers,
	                                               VkPipelineStageFlags SourceStage,
	                                               VkPipelineStageFlags DestinationStage)
	{
		vkCmdPipelineBarrier(Handle, SourceStage, DestinationStage, 0, 0, nullptr, static_cast<uint32>(Barriers.size()),
		                     Barriers.data(), 0, nullptr);
	}

	void CommandBuffer::InsertImageMemoryBarrier(const VkImageMemoryBarrier& Barrier,
	                                             VkPipelineStageFlags SourceStage,
	                                             VkPipelineStageFlags DestinationStage)
	{
		vkCmdPipelineBarrier(Handle, SourceStage, DestinationStage, 0, 0, nullptr, 0, nullptr, 1, &Barrier);
	}

	void CommandBuffer::InsertImageMemoryBarriers(std::span<const VkImageMemoryBarrier> Barriers,
	                                              VkPipelineStageFlags SourceStage,
	                                              VkPipelineStageFlags DestinationStage)
	{
		vkCmdPipelineBarrier(Handle, SourceStage, DestinationStage, 0, 0, nullptr, 0, nullptr,
		                     static_cast<uint32>(Barriers.size()), Barriers.data());
	}

	void CommandBuffer::CopyBuffer(const Buffer& Source, const Buffer& Destination, std::span<VkBufferCopy> CopyRegions)
	{
		vkCmdCopyBuffer(Handle, Source.GetBuffer(), Destination.GetBuffer(), static_cast<uint32>(CopyRegions.size()),
		                CopyRegions.data());
	}

	void CommandBuffer::CopyBufferToImage(const Buffer& Source, const Image& Destination,
	                                      VkImageLayout DestinationImageLayout,
	                                      std::span<VkBufferImageCopy> CopyRegions)
	{
		vkCmdCopyBufferToImage(Handle, Source.GetBuffer(), Destination.GetImage(), DestinationImageLayout,
		                       static_cast<uint32>(CopyRegions.size()), CopyRegions.data());
	}

	void CommandBuffer::CopyImage(const Image& Source, VkImageLayout SourceLayout, const Image& Destination,
	                              VkImageLayout DestinationLayout, std::span<VkImageCopy> CopyRegions)
	{
		vkCmdCopyImage(Handle, Source.GetImage(), SourceLayout, Destination.GetImage(), DestinationLayout,
		               static_cast<uint32>(CopyRegions.size()), CopyRegions.data());
	}

	void CommandBuffer::BlitImage(const Image& Source, VkImageLayout SourceLayout, const Image& Destination,
	                              VkImageLayout DestinationLayout, std::span<VkImageBlit> Regions, VkFilter Filter)
	{
		vkCmdBlitImage(Handle, Source.GetImage(), SourceLayout, Destination.GetImage(), DestinationLayout,
		               static_cast<uint32>(Regions.size()), Regions.data(), Filter);
	}

	VkCommandBuffer CommandBuffer::GetBuffer() const
	{
		return Handle;
	}
}