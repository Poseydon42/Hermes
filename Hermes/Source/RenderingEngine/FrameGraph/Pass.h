#pragma once

#include <unordered_map>
#include <variant>

#include "Core/Core.h"
#include "Core/Delegate/Delegate.h"
#include "Vulkan/Forward.h"

namespace Hermes
{
	struct Attachment;
	struct FrameMetrics;
	class GeometryList;
	class Scene;

	enum class PassType
	{
		Graphics,
		Compute
	};

	using PassResourceVariant = std::variant<const Vulkan::Buffer*, const Vulkan::ImageView*>;

	struct PassCallbackInfo
	{
		Vulkan::CommandBuffer& CommandBuffer;
		Vulkan::RenderPass* RenderPass;

		const std::unordered_map<String, PassResourceVariant>& Resources;

		const Scene& Scene;
		const GeometryList& GeometryList;

		FrameMetrics& Metrics;

		bool ResourcesWereChanged;
	};

	struct PassDesc
	{
		using PassCallbackType = TDelegate<void, const PassCallbackInfo&>;

		std::vector<Attachment> Attachments;

		PassCallbackType Callback;

		PassType Type = PassType::Graphics;
	};
}
