#pragma once

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"

namespace Hermes
{
	namespace RenderInterface
	{
		class HERMES_API Swapchain
		{
			MAKE_NON_COPYABLE(Swapchain)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(Swapchain)
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Swapchain)
			ADD_DEFAULT_CONSTRUCTOR(Swapchain)
		};
	}
}
