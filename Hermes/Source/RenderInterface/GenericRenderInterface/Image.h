#pragma once

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/EnumClassOperators.h"
#include "RenderInterface/GenericRenderInterface/CommonTypes.h"
#include "Math/Vector2.h"

namespace Hermes
{
	namespace RenderInterface
	{
		enum class ImageUsageType
		{
			Sampled = 1 << 0,
			ColorAttachment = 1 << 1,
			DepthStencilAttachment = 1 << 2,
			InputAttachment = 1 << 3,
			CopySource = 1 << 4,
			CopyDestination = 1 << 5,
			CPUAccessible = 1 << 6,
		};

		ENUM_CLASS_OPERATORS(ImageUsageType)

		class HERMES_API Image
		{
			ADD_DEFAULT_CONSTRUCTOR(Image);
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Image);
			ADD_DEFAULT_MOVE_CONSTRUCTOR(Image);
			MAKE_NON_COPYABLE(Image);

		public:
			virtual Vec2ui GetSize() const = 0;

			virtual DataFormat GetDataFormat() const = 0;

			virtual uint32 GetMipLevelsCount() const = 0;

			virtual ImageUsageType GetUsageFlags() const = 0;
		};
	}
}
