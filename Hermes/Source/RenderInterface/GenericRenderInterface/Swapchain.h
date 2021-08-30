#pragma once

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"
#include "RenderInterface/GenericRenderInterface/CommonTypes.h"
#include "Math/Vector2.h"

namespace Hermes
{
	namespace RenderInterface
	{
		class Image;
		
		class HERMES_API Swapchain
		{
			MAKE_NON_COPYABLE(Swapchain)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(Swapchain)
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Swapchain)
			ADD_DEFAULT_CONSTRUCTOR(Swapchain)

		public:
			virtual DataFormat GetImageFormat() const = 0;

			virtual Vec2ui GetSize() const = 0;

			virtual std::shared_ptr<Image> GetImage(uint32 Index) const = 0;

			virtual uint32 GetImageCount() const = 0;
		};
	}
}
