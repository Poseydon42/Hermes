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

		struct ImageViewDescription
		{
			ImageAspect Aspects;
			uint32 BaseMipLevel;
			uint32 MipLevelCount;
		};

		class HERMES_API ImageView
		{
			ADD_DEFAULT_CONSTRUCTOR(ImageView);
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(ImageView);
			ADD_DEFAULT_MOVE_CONSTRUCTOR(ImageView);
			MAKE_NON_COPYABLE(ImageView);
		};

		class HERMES_API Image
		{
			ADD_DEFAULT_CONSTRUCTOR(Image);
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Image);
			ADD_DEFAULT_MOVE_CONSTRUCTOR(Image);
			MAKE_NON_COPYABLE(Image);

		public:
			virtual std::unique_ptr<ImageView> CreateImageView(const ImageViewDescription& Description) const = 0;

			virtual std::unique_ptr<ImageView> CreateDefaultImageView() const = 0;

			virtual Vec2ui GetSize() const = 0;

			virtual DataFormat GetDataFormat() const = 0;

			virtual uint32 GetMipLevelsCount() const = 0;

			virtual ImageUsageType GetUsageFlags() const = 0;
		};
	}
}
