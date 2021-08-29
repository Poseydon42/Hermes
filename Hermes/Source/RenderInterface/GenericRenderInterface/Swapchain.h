#pragma once

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"
#include "RenderInterface/GenericRenderInterface/Image.h"
#include "Math/Vector2.h"

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

		public:
			virtual DataFormat GetImageFormat() const = 0;

			virtual Vec2ui GetSize() const = 0;
		};
	}
}
