#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderingEngine/FrameGraph/Pass.h"

namespace Hermes
{
	class HERMES_API DepthPass
	{
	public:
		DepthPass();

		const PassDesc& GetPassDescription() const;
	private:
		std::unique_ptr<Vulkan::Buffer> SceneUBO;
		std::unique_ptr<Vulkan::DescriptorSet> SceneUBODescriptorSet;

		PassDesc Description;

		void PassCallback(Vulkan::CommandBuffer& CommandBuffer, const Vulkan::RenderPass& PassInstance,
		                  const std::vector<std::pair<const Vulkan::Image*, const Vulkan::ImageView*>>& Attachments,
		                  const Scene& Scene, const GeometryList& GeometryList, FrameMetrics& Metrics,
		                  bool ResourcesWereRecreated);
	};
}
