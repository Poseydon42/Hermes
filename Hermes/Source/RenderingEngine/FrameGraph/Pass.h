#pragma once

#include "Core/Core.h"
#include "Core/Delegate/Delegate.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"

namespace Hermes
{
	struct Attachment;
	struct FrameMetrics;
	class GeometryList;
	class Scene;

	struct PassDesc
	{
		/*
		 * void Callback(RenderInterface::CommandBuffer& TargetCommandBuffer, const RenderInterface::RenderPass& PassInstance, const std::vector<std::pair<const RenderInterface::Image*, const RenderInterface::ImageView*>>& Attachments, const Scene& Scene, const GeometryList& GeometryList, FrameMetrics& Metrics, bool ResourcesWereChanged)
		 */
		using RenderPassCallbackType = TDelegate<void,
			RenderInterface::CommandBuffer&, const RenderInterface::RenderPass&,
			const std::vector<std::pair<const RenderInterface::Image*, const RenderInterface::ImageView*>>&,
			const Scene&, const GeometryList&, FrameMetrics&, bool>;

		std::vector<Attachment> Attachments;
		RenderPassCallbackType Callback;
	};
}
