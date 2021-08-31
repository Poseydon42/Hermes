#pragma once

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	namespace RenderInterface
	{
		class HERMES_API RenderTarget
		{
			ADD_DEFAULT_CONSTRUCTOR(RenderTarget);
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(RenderTarget);
			ADD_DEFAULT_MOVE_CONSTRUCTOR(RenderTarget);
			MAKE_NON_COPYABLE(RenderTarget);
		};
	}
}
