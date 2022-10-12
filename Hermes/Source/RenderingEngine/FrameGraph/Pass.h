#pragma once

#include "Core/Core.h"
#include "Core/Delegate/Delegate.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"

namespace Hermes
{
	struct FrameMetrics;
	class Scene;
	struct Attachment;

	struct PassDesc
	{
		/*
		 * void Callback(RenderInterface::CommandBuffer& TargetCommandBuffer, const RenderInterface::RenderPass& PassInstance, const std::vector<std::pair<const RenderInterface::Image*, const RenderInterface::ImageView*>>& Attachments, const Scene& Scene, FrameMetrics& Metrics, bool ResourcesWereChanged)
		 */
		using RenderPassCallbackType = TDelegate<void,
			RenderInterface::CommandBuffer&, const RenderInterface::RenderPass&,
			const std::vector<std::pair<const RenderInterface::Image*, const RenderInterface::ImageView*>>&,
			const Scene&, FrameMetrics&, bool>;

		std::vector<Attachment> Attachments;
		RenderPassCallbackType Callback;
	};
}
