#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	namespace RenderInterface
	{
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
			
			/*************************************
			 *      Transfer queue commands      *
			 *************************************/
			virtual void CopyBuffer(const std::shared_ptr<Buffer>& Source, const std::shared_ptr<Buffer>& Destination, std::vector<BufferCopyRegion> CopyRegions) = 0;
		};
	}
}
