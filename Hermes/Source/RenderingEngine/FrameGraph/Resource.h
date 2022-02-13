#pragma once

#include "Core/Core.h"
#include "Math/Math.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"

namespace Hermes
{
	class HERMES_API SwapchainRelativeDimensions
	{
	public:
		SwapchainRelativeDimensions();

		static SwapchainRelativeDimensions CreateFromAbsoluteDimensions(Vec2ui Dimensions);

		static SwapchainRelativeDimensions CreateFromRelativeDimensions(Vec2 Dimensions);

		Vec2ui GetAbsoluteDimensions(Vec2ui SwapchainDimensions) const;

		bool IsRelative() const;

		bool IsAbsolute() const;
	private:
		SwapchainRelativeDimensions(Vec2ui AbsoluteDimensions);

		SwapchainRelativeDimensions(Vec2 RelativeDimensions);

		enum class ValueType
		{
			Relative,
			Absolute
		} Type;
		union
		{
			Vec2 Relative;
			Vec2ui Absolute;
		};
	};

	struct ResourceDesc
	{
		RenderInterface::DataFormat Format;
		SwapchainRelativeDimensions Dimensions;
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
