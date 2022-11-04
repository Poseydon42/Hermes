#pragma once

#include "Core/Core.h"
#include "Core/Delegate/Delegate.h"
#include "Vulkan/Forward.h"

namespace Hermes
{
	struct Attachment;
	struct FrameMetrics;
	class GeometryList;
	class Scene;

	struct PassDesc
	{
		/*
		 * void Callback(Vulkan::CommandBuffer& TargetCommandBuffer, const Vulkan::RenderPass& PassInstance, const std::vector<std::pair<const Vulkan::Image*, const Vulkan::ImageView*>>& Attachments, const Scene& Scene, const GeometryList& GeometryList, FrameMetrics& Metrics, bool ResourcesWereChanged)
		 */
		using RenderPassCallbackType = TDelegate<void,
			Vulkan::CommandBuffer&, const Vulkan::RenderPass&,
			const std::vector<std::pair<const Vulkan::Image*, const Vulkan::ImageView*>>&,
			const Scene&, const GeometryList&, FrameMetrics&, bool>;

		std::vector<Attachment> Attachments;
		RenderPassCallbackType Callback;
	};
}
