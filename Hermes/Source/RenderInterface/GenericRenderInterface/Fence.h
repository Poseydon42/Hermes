#pragma once

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	namespace RenderInterface
	{
		/**
		 * A binary synchronization primitive
		 * Could be either signaled('On') or not signaled('Off')
		 * Its state could be read from CPU side, waited for it to become signaled and reset, but can be signaled
		 * only from GPU side during command execution
		 */
		class HERMES_API Fence
		{
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Fence)
			ADD_DEFAULT_CONSTRUCTOR(Fence)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(Fence)
			MAKE_NON_COPYABLE(Fence)

		public:

			virtual bool IsSignaled() const = 0;

			/**
			 * Waits for given fence to become signaled for @param Timeout seconds
			 * @param Timeout Maximum time(in seconds) to wait for fence(UINT64_MAX for unlimited time)
			 */
			virtual void Wait(uint64 Timeout) const = 0;

			virtual void Reset() = 0;
		};
	}
}
