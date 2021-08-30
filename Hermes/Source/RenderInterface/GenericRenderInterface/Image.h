#pragma once

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "RenderInterface/GenericRenderInterface/CommonTypes.h"
#include "Math/Vector2.h"

namespace Hermes
{
	namespace RenderInterface
	{
		class HERMES_API Image
		{
			ADD_DEFAULT_CONSTRUCTOR(Image);
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Image);
			ADD_DEFAULT_MOVE_CONSTRUCTOR(Image);
			MAKE_NON_COPYABLE(Image);

		public:
			virtual Vec2ui GetSize() const = 0;

			virtual DataFormat GetDataFormat() const = 0;
		};
	}
}
