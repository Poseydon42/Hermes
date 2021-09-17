#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	namespace RenderInterface
	{
		class DescriptorSet;
		class Pipeline;
		class RenderTarget;
		class RenderPass;
		class Buffer;

		struct BufferCopyRegion
		{
			size_t SourceOffset;
			size_t DestinationOffset;
			size_t NumBytes;
		};

		struct ClearColor
		{
			float R;
			float G;
			float B;
			float A;
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
			virtual void BeginRenderPass(const std::shared_ptr<RenderPass>& RenderPass, const std::shared_ptr<RenderTarget>& RenderTarget, const std::vector<ClearColor>& ClearColors) = 0;

			/**
			 * Ends currently active render pass
			 */
			virtual void EndRenderPass() = 0;

			virtual void BindPipeline(const std::shared_ptr<Pipeline>& Pipeline) = 0;

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
			
			/*************************************
			 *      Transfer queue commands      *
			 *************************************/
			virtual void CopyBuffer(const std::shared_ptr<Buffer>& Source, const std::shared_ptr<Buffer>& Destination, std::vector<BufferCopyRegion> CopyRegions) = 0;
		};
	}
}
