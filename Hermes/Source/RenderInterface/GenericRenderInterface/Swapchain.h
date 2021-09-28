#pragma once

#include <optional>

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
		class Fence;
		
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

			/**
			 * Returns index of next available image, if exists
			 * @param Timeout Maximal time in nanoseconds that function would wait for image
			 * @param Fence Fence to signal when image could be used by user side
			 * @param SwapchainWasRecreated Will be set to true if swapchain was recreated due to its mismatch with window format and/or size
			 */
			virtual std::optional<uint32> AcquireImage(uint64 Timeout, const Fence& Fence, bool& SwapchainWasRecreated) = 0;

			virtual void Present(uint32 ImageIndex, bool& SwapchainWasRecreated) = 0;
		};
	}
}
