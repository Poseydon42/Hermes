#pragma once

#include "Core/Core.h"
#include "Math/Math.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes
{
	class HERMES_API ViewportRelativeDimensions
	{
	public:
		ViewportRelativeDimensions();

		static ViewportRelativeDimensions CreateFromAbsoluteDimensions(Vec2ui Dimensions);

		static ViewportRelativeDimensions CreateFromRelativeDimensions(Vec2 Dimensions);

		Vec2ui GetAbsoluteDimensions(Vec2ui SwapchainDimensions) const;

		bool IsRelative() const;

		bool IsAbsolute() const;
	private:
		ViewportRelativeDimensions(Vec2ui AbsoluteDimensions);

		ViewportRelativeDimensions(Vec2 RelativeDimensions);

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
		ViewportRelativeDimensions Dimensions;
		uint32 MipLevels = 0;
	};

	struct BufferResourceDescription
	{
		uint32 Size = 0;
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
		VkBufferUsageFlags Usage = 0;
		bool RequiresMapping = false;
	};
}
