#pragma once

#include "Core/Core.h"
#include "Core/Delegate/Delegate.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"

namespace Hermes
{
	class Scene;
	struct Source;
	struct Drain;

	struct PassDesc
	{
		/*
		 * void Callback(RenderInterface::CommandBuffer& TargetCommandBuffer, const RenderInterface::RenderPass& PassInstance, const std::vector<std::pair<const RenderInterface::Image*, const RenderInterface::ImageView*>>& DrainAttachments, const Scene& Scene, bool ResourcesWereChanged)
		 */
		using RenderPassCallbackType = TDelegate<void, RenderInterface::CommandBuffer&, const RenderInterface::RenderPass&,
			const std::vector<std::pair<const RenderInterface::Image*, const RenderInterface::ImageView*>>&, const Scene&, bool>;

		std::vector<Drain> Drains;
		std::vector<Source> Sources;
		RenderPassCallbackType Callback;
	};
}
