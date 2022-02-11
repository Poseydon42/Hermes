#pragma once

#include "Core/Core.h"
#include "Math/Math.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"

namespace Hermes
{
	struct ResourceDesc
	{
		RenderInterface::DataFormat Format;
		Vec2ui Dimensions;
		uint32 MipLevels;
	};

	// TODO : input attachment binding support
	enum class BindingMode
	{
		ColorAttachment,
		DepthStencilAttachment
	};

	struct Drain
	{
		String Name;
		RenderInterface::DataFormat Format;
		RenderInterface::AttachmentLoadOp LoadOp;
		RenderInterface::AttachmentLoadOp StencilLoadOp;
		float ClearColor[4];
		RenderInterface::ImageLayout Layout;
		BindingMode Binding;
	};

	struct Source
	{
		String Name;
		RenderInterface::DataFormat Format;
	};
}
