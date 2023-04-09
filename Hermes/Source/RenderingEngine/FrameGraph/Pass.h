#pragma once

#include <functional>
#include <unordered_map>
#include <variant>

#include "Core/Core.h"
#include "Vulkan/Forward.h"

namespace Hermes
{
	struct Attachment;
	struct BufferInput;
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

		const std::unordered_map<String, PassResourceVariant>& Resources;

		const Scene& Scene;
		const GeometryList& GeometryList;

		FrameMetrics& Metrics;
	};

	struct PassDesc
	{
		using PassCallbackType = std::function<void(const PassCallbackInfo&)>;

		std::vector<Attachment> Attachments;
		std::vector<BufferInput> BufferInputs;

		PassCallbackType Callback;

		PassType Type = PassType::Graphics;
	};
}
