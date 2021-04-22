#pragma once

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	namespace RenderInterface
	{
		/**
		 * Represents a 'logical device', basically an interface to all GPU functionality
		 */
		class HERMES_API Device
		{
			MAKE_NON_COPYABLE(Device)
		public:
			Device() = default;
			virtual ~Device() = default;
			Device(Device&&) = default;
			Device& operator=(Device&&) = default;
		};
	}
}
