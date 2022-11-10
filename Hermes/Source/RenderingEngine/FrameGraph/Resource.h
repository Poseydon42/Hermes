#pragma once

#include "Core/Core.h"
#include "Math/Math.h"
#include "Vulkan/VulkanCore.h"

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

	struct ImageResourceDescription
	{
		VkFormat Format = VK_FORMAT_UNDEFINED;
		SwapchainRelativeDimensions Dimensions;
		uint32 MipLevels = 0;
	};

	enum class BindingMode : uint32_t
	{
		InputAttachment = 0,
		ColorAttachment = 1,
		DepthStencilAttachment = 2,
	};

	struct Attachment
	{
		String Name;
		VkAttachmentLoadOp LoadOp;
		VkAttachmentLoadOp StencilLoadOp;
		VkClearValue ClearColor;
		BindingMode Binding;
	};

	struct BufferInput
	{
		String Name;
		bool ClearBeforePass = false;
	};
}
