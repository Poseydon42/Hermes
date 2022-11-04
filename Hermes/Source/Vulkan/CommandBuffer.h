#pragma once

#include <memory>
#include <span>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Vulkan/Forward.h"
#include "Vulkan/Queue.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes::Vulkan
{
	/*
	 * A safe wrapper around VkCommandBuffer
	 */
	class CommandBuffer
	{
		MAKE_NON_COPYABLE(CommandBuffer)
		MAKE_NON_MOVABLE(CommandBuffer)

	public:
		CommandBuffer(std::shared_ptr<Queue::VkQueueHolder> InQueue, bool IsPrimaryBuffer);

		~CommandBuffer();

		void BeginRecording();

		void EndRecording();

		void BeginRenderPass(const RenderPass& RenderPass, const Framebuffer& Framebuffer,
		                     std::span<VkClearValue> ClearColors);

		void EndRenderPass();

		void BindPipeline(const Pipeline& Pipeline);

		void Draw(uint32 VertexCount, uint32 InstanceCount, uint32 VertexOffset, uint32 InstanceOffset);

		void DrawIndexed(uint32 IndexCount, uint32 InstanceCount, uint32 IndexOffset, int32 VertexOffset,
		                 uint32 InstanceOffset);

		void BindVertexBuffer(const Buffer& Buffer);

		void BindIndexBuffer(const Buffer& Buffer, VkIndexType IndexType);

		void BindDescriptorSet(const DescriptorSet& Set, const Pipeline& Pipeline, uint32 BindingIndex);

		void UploadPushConstants(const Pipeline& Pipeline, VkShaderStageFlags ShadersThatUse, const void* Data,
		                         uint32 Size, uint32 Offset);
		
		void InsertBufferMemoryBarrier(const VkBufferMemoryBarrier& Barrier, VkPipelineStageFlags SourceStage,
		                               VkPipelineStageFlags DestinationStage);

		void InsertBufferMemoryBarriers(std::span<VkBufferMemoryBarrier> Barriers, VkPipelineStageFlags SourceStage,
		                               VkPipelineStageFlags DestinationStage);

		void InsertImageMemoryBarrier(const VkImageMemoryBarrier& Barrier, VkPipelineStageFlags SourceStage,
		                              VkPipelineStageFlags DestinationStage);

		void InsertImageMemoryBarriers(std::span<const VkImageMemoryBarrier> Barriers, VkPipelineStageFlags SourceStage,
		                              VkPipelineStageFlags DestinationStage);

		void CopyBuffer(const Buffer& Source, const Buffer& Destination, std::span<VkBufferCopy> CopyRegions);

		void CopyBufferToImage(const Buffer& Source, const Image& Destination, VkImageLayout DestinationImageLayout,
		                       std::span<VkBufferImageCopy> CopyRegions);

		void CopyImage(const Image& Source, VkImageLayout SourceLayout, const Image& Destination,
		               VkImageLayout DestinationLayout, std::span<VkImageCopy> CopyRegions);

		void BlitImage(const Image& Source, VkImageLayout SourceLayout, const Image& Destination,
		               VkImageLayout DestinationLayout, std::span<VkImageBlit> Regions, VkFilter Filter);

		VkCommandBuffer GetBuffer() const;

	private:
		std::shared_ptr<Queue::VkQueueHolder> Queue;

		VkCommandBuffer Handle = VK_NULL_HANDLE;
	};
}
