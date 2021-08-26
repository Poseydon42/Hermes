#pragma once

#include "Core/Core.h"

namespace Hermes
{
	namespace RenderInterface
	{
		using ImageFormat = int64; // TODO : Change it to some kind of API-agnostic enum instead of opaque handle

		enum class ImageLayout
		{
			Undefined,
			General,
			ColorAttachmentOptimal,
			DepthAttachmentOptimal,
			DepthReadOnlyOptimal,
			StencilAttachmentOptimal,
			StencilReadOnlyOptimal,
			DepthStencilAttachmentOptimal,
			DepthStencilReadOnlyOptimal,
			DepthReadOnlyStencilAttachmentOptimal,
			DepthAttachmentStencilReadOnlyOptimal,
			ShaderReadOnlyOptimal,
			TransferSourceOptimal,
			TransferDestinationOptimal,
			Preinitialized,
			ReadyForPresentation
		};
	}
}
