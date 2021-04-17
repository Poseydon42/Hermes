#pragma once

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	namespace RenderInterface
	{
		/**
		 * This is a main access point to all rendering API functionality
		 * It is responsible for enumerating available devices and creating them
		 */
		class Instance
		{
			MAKE_NON_COPYABLE(Instance)
		
		public:
			virtual ~Instance() = default;
			Instance(Instance&& Other) = default;
			Instance& operator=(Instance&& Other) = default;
		};
	}
}

