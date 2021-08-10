#pragma once

#include <optional>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/EnumClassOperators.h"

namespace Hermes
{
	namespace RenderInterface
	{
		class Fence;
		class CommandBuffer;

		enum class QueueType
		{
			Render,
			Transfer,
			Presentation
		};

		ENUM_CLASS_OPERATORS(QueueType)
		
		class HERMES_API Queue
		{
		public:
			ADD_DEFAULT_CONSTRUCTOR(Queue)
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Queue)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(Queue)
			MAKE_NON_COPYABLE(Queue)

			/**
			 * Creates a command buffer that can be executed on this queue later
			 * @param IsPrimaryBuffer If true then this command buffer can be submitted to queue, but can not be called from other command buffers \n
			 * If false then this command buffer can be called from other command buffers, but can not be submitted to queue directly
			 */
			virtual std::shared_ptr<CommandBuffer> CreateCommandBuffer(bool IsPrimaryBuffer) = 0;

			/**
			 * Pushes given command buffer into queue's internal 'execution list'
			 * @param Buffer A command buffer to be executed
			 * @param Fence An optional fence object that needs to be signaled when GPU finishes execution of given command buffer
			 */
			virtual void SubmitCommandBuffer(std::shared_ptr<CommandBuffer> Buffer, std::optional<std::shared_ptr<Fence>> Fence) = 0;
		};
	}
}
