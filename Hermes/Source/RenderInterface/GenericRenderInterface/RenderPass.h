#pragma once

#include <optional>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/EnumClassOperators.h"
#include "RenderInterface/GenericRenderInterface/CommonTypes.h"

#include <vector>

namespace Hermes
{
	namespace RenderInterface
	{
		enum class AttachmentLoadOp
		{
			Load, // Attachment content is preserved
			Clear, // Attachment content is cleared
			Undefined // Don't care about attachment content, its state is undefined
		};

		enum class AttachmentStoreOp
		{
			Store, // Attachment content is stored
			Undefined // Don't care about attachment content, its state is undefined
		};

		enum class AttachmentType
		{
			Color,
			DepthStencil,
			Input
		};

		struct RenderPassAttachment
		{
			AttachmentType Type;
			AttachmentLoadOp LoadOp;
			AttachmentStoreOp StoreOp;
			AttachmentLoadOp StencilLoadOp;
			AttachmentStoreOp StencilStoreOp;
			DataFormat Format;
			ImageLayout LayoutAtStart;
			ImageLayout LayoutAtEnd;
			// TODO : sample count, flags
		};
		
		class HERMES_API RenderPass
		{
			ADD_DEFAULT_CONSTRUCTOR(RenderPass)
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(RenderPass)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(RenderPass)
			MAKE_NON_COPYABLE(RenderPass)

		public:

			virtual uint32 GetColorAttachmentCount() const = 0;
		};
	}
}
