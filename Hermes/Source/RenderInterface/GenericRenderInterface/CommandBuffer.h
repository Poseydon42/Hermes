#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	namespace RenderInterface
	{
		class Buffer;

		struct BufferCopyRegion
		{
			size_t SourceOffset;
			size_t DestinationOffset;
			size_t NumBytes;
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
			
			/**
			 * Transfer queue commands
			 */
			virtual void CopyBuffer(const std::shared_ptr<Buffer>& Source, const std::shared_ptr<Buffer>& Destination, std::vector<BufferCopyRegion> CopyRegions) = 0;
		};
	}
}
