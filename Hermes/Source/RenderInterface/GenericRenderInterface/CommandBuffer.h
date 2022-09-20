#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "RenderInterface/GenericRenderInterface/CommonTypes.h"
#include "Math/Math.h"

namespace Hermes
{
	namespace RenderInterface
	{
		enum class ImageAspect;
		enum class FilteringMode;
		enum class ShaderType;
		class Queue;
		enum class ImageLayout;
		class Image;
		class DescriptorSet;
		class Pipeline;
		class RenderTarget;
		class RenderPass;
		class Buffer;

		struct ImageMemoryBarrier
		{
			ImageLayout OldLayout { ImageLayout::Undefined };
			ImageLayout NewLayout { ImageLayout::Undefined };
			std::optional<const Queue*> OldOwnerQueue {};
			std::optional<const Queue*> NewOwnerQueue {};
			AccessType OperationsThatHaveToEndBefore { AccessType::None };
			AccessType OperationsThatCanStartAfter { AccessType::None };
			uint32 BaseMipLevel = 0;
			uint32 MipLevelCount = 0;
			std::optional<CubemapSide> Side;
		};

		struct BufferMemoryBarrier
		{
			std::optional<const Queue*> OldOwnerQueue {};
			std::optional<const Queue*> NewOwnerQueue {};
			AccessType OperationsThatHaveToEndBefore { AccessType::None };
			AccessType OperationsThatCanStartAfter { AccessType::None };
			size_t Offset { 0 };
			size_t NumBytes { 0 };
		};

		struct BufferCopyRegion
		{
			size_t SourceOffset;
			size_t DestinationOffset;
			size_t NumBytes;
		};

		struct BufferToImageCopyRegion
		{
			uint32 BufferOffset;
			std::optional<uint32> RowLengthInBuffer;
			Vec2ui ImageOffset;
			Vec2ui ImageDimensions;
			uint32 MipLevel;
			std::optional<CubemapSide> Side;
		};

		struct ImageCopyRegion
		{
			Vec2i SourceOffset;
			uint32 SourceMipLayer;
			std::optional<CubemapSide> SourceSide;
			Vec2i DestinationOffset;
			uint32 DestinationMipLayer;
			std::optional<CubemapSide> DestinationSide;
			Vec2ui Dimensions;
		};

		struct ImageBlitRegion
		{
			struct
			{
				ImageAspect AspectMask;
				uint32 MipLevel;
				Vec2ui RectMin;
				Vec2ui RectMax;
				std::optional<CubemapSide> Side;
			} SourceRegion, DestinationRegion;
		};

		struct ClearColor
		{
			float R = 0.0f;
			float G = 0.0f;
			float B = 0.0f;
			float A = 0.0f;
			float Depth = 0.0f;
			uint32 Stencil = 0;
		};

		enum class IndexSize
		{
			Uint16,
			Uint32
		};
		
		class HERMES_API CommandBuffer
		{
			ADD_DEFAULT_CONSTRUCTOR(CommandBuffer)
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(CommandBuffer)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(CommandBuffer)
			MAKE_NON_COPYABLE(CommandBuffer)

		public:
			/*
			 * Resets all previously recorded commands and enabled recording again
			 */
			virtual void BeginRecording() = 0;

			/**
			 * Ends recording of buffer
			 */
			virtual void EndRecording() = 0;

			/*************************************
			 *       Render queue commands       *
			 *************************************/

			/*
			 * Begins a given render pass with given render target attached
			 * @param ClearColors Array of colors with size equal to number of attachments of render pass, filled with color values that will be used to clear attachments
			 */
			virtual void BeginRenderPass(const RenderPass& RenderPass, const RenderTarget& RenderTarget, const std::vector<ClearColor>& ClearColors) = 0;

			/**
			 * Ends currently active render pass
			 */
			virtual void EndRenderPass() = 0;

			virtual void BindPipeline(const Pipeline& Pipeline) = 0;

			/*
			 * Makes a non-indexed draw call
			 * @param VertexOffset Index of first vertex in vertex buffer to draw
			 * @param InstanceOffset ID of first drawn instance
			 */
			virtual void Draw(uint32 VertexCount, uint32 InstanceCount, uint32 VertexOffset, uint32 InstanceOffset) = 0;

			/*
			 * Makes an indexed draw call
			 * @param IndexOffset Index of first index in index buffer to draw
			 * @param VertexOffset A value added to index value before indexing into vertex buffer
			 * @param InstanceOffset ID of first drawn instance
			 */
			virtual void DrawIndexed(uint32 IndexCount, uint32 InstanceCount, uint32 IndexOffset, uint32 VertexOffset, uint32 InstanceOffset) = 0;

			// TODO : multi-buffer version
			// TODO : offsets
			/*
			 * Binds a vertex buffer for further drawing commands
			 */
			virtual void BindVertexBuffer(const Buffer& Buffer) = 0;
			
			/*
			 * Binds an index buffer for further drawing commands
			 */
			virtual void BindIndexBuffer(const Buffer& Buffer, IndexSize Size) = 0;

			// TODO : multi-set version
			/*
			 * Binds a descriptor set for further drawing commands
			 */
			virtual void BindDescriptorSet(const DescriptorSet& Set, const Pipeline& Pipeline, uint32 BindingIndex) = 0;

			/*
			 * Uploads push constants data onto GPU
			 * This data is valid until the next call to this function or end of render pass
			 */
			virtual void UploadPushConstants(const Pipeline& Pipeline, ShaderType ShadersThatUse, const void* Data, uint32 Size, uint32 Offset) = 0;

			// TODO : generic, multi-barrier version
			/*
			 * Inserts a buffer memory barrier
			 */
			virtual void InsertBufferMemoryBarrier(const Buffer& Buffer, const BufferMemoryBarrier& Barrier, PipelineStage SourceStage, PipelineStage DestinationStage) = 0;

			/*
			 * Inserts an image memory barrier
			 */
			virtual void InsertImageMemoryBarrier(const Image& Image, const ImageMemoryBarrier& Barrier, PipelineStage SourceStage, PipelineStage DestinationStage) = 0;
			
			/*************************************
			 *      Transfer queue commands      *
			 *************************************/
			virtual void CopyBuffer(const Buffer& Source, const Buffer& Destination, std::vector<BufferCopyRegion> CopyRegions) = 0;

			virtual void CopyBufferToImage(const Buffer& Source, const Image& Destination, ImageLayout DestinationImageLayout, std::vector<BufferToImageCopyRegion> CopyRegions) = 0;

			virtual void CopyImage(const Image& Source, ImageLayout SourceLayout, const Image& Destination, ImageLayout DestinationLayout, std::vector<ImageCopyRegion> CopyRegions) = 0;

			virtual void BlitImage(
				const Image& Source, ImageLayout SourceLayout, const Image& Destination, ImageLayout DestinationLayout,
				const std::vector<ImageBlitRegion>& Regions, FilteringMode Filter) = 0;
		};
	}
}
