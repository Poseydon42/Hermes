#pragma once

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/CommandBuffer.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanDevice;

		class VulkanCommandBuffer : public RenderInterface::CommandBuffer
		{
		public:
			MAKE_NON_COPYABLE(VulkanCommandBuffer)

			VulkanCommandBuffer(std::shared_ptr<VulkanDevice> InDevice, VkCommandPool InPool, bool IsPrimaryBuffer);

			~VulkanCommandBuffer() override;

			VulkanCommandBuffer(VulkanCommandBuffer&& Other);
			VulkanCommandBuffer& operator=(VulkanCommandBuffer&& Other);

			void BeginRecording() override;
			
			void EndRecording() override;
			
			void BeginRenderPass(const std::shared_ptr<RenderInterface::RenderPass>& RenderPass, const std::shared_ptr<RenderInterface::RenderTarget>& RenderTarget, const std::vector<RenderInterface::ClearColor>& ClearColors) override;
			
			void EndRenderPass() override;

			void BindPipeline(const std::shared_ptr<RenderInterface::Pipeline>& Pipeline) override;

			void Draw(uint32 VertexCount, uint32 InstanceCount, uint32 VertexOffset, uint32 InstanceOffset) override;
			
			void DrawIndexed(uint32 IndexCount, uint32 InstanceCount, uint32 IndexOffset, uint32 VertexOffset, uint32 InstanceOffset) override;
			
			void BindVertexBuffer(const RenderInterface::Buffer& Buffer) override;
			
			void BindIndexBuffer(const RenderInterface::Buffer& Buffer, RenderInterface::IndexSize Size) override;

			void BindDescriptorSet(const RenderInterface::DescriptorSet& Set, const RenderInterface::Pipeline& Pipeline, uint32 BindingIndex) override;

			void UploadPushConstants(const RenderInterface::Pipeline& Pipeline, RenderInterface::ShaderType ShadersThatUse, const void* Data, uint32 Size, uint32 Offset) override;

			void InsertBufferMemoryBarrier(const RenderInterface::Buffer& Buffer, const RenderInterface::BufferMemoryBarrier& Barrier, RenderInterface::PipelineStage SourceStage, RenderInterface::PipelineStage DestinationStage) override;

			void InsertImageMemoryBarrier(const RenderInterface::Image& Image, const RenderInterface::ImageMemoryBarrier& Barrier, RenderInterface::PipelineStage SourceStage, RenderInterface::PipelineStage DestinationStage) override;
			
			void CopyBuffer(const RenderInterface::Buffer& Source, const RenderInterface::Buffer& Destination, std::vector<RenderInterface::BufferCopyRegion> CopyRegions) override;

			void CopyBufferToImage(const RenderInterface::Buffer& Source, const RenderInterface::Image& Destination, RenderInterface::ImageLayout DestinationImageLayout, std::vector<RenderInterface::BufferToImageCopyRegion> CopyRegions) override;

			void CopyImage(const RenderInterface::Image& Source, RenderInterface::ImageLayout SourceLayout, const RenderInterface::Image& Destination, RenderInterface::ImageLayout DestinationLayout, std::vector<RenderInterface::ImageCopyRegion> CopyRegions) override;

			VkCommandBuffer GetBuffer() const { return Buffer; }

		private:
			VkCommandBuffer Buffer;
			VkCommandPool Pool;
			std::shared_ptr<VulkanDevice> Device;
		};
	}
}
