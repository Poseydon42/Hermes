#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"

namespace Hermes
{
	class HERMES_API PostProcessingPass
	{
		MAKE_NON_COPYABLE(PostProcessingPass)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(PostProcessingPass)
		ADD_DEFAULT_DESTRUCTOR(PostProcessingPass)

	public:
		PostProcessingPass();

		const PassDesc& GetPassDescription() const;

	private:
		std::shared_ptr<RenderInterface::DescriptorSetLayout> DescriptorLayout;
		std::unique_ptr<RenderInterface::DescriptorSet> DescriptorSet;

		bool IsPipelineCreated = false;
		std::unique_ptr<RenderInterface::Pipeline> Pipeline;
		std::shared_ptr<RenderInterface::Shader> VertexShader, FragmentShader;

		PassDesc Description;

		void PassCallback(
			RenderInterface::CommandBuffer& CommandBuffer,
			const RenderInterface::RenderPass& PassInstance,
			const std::vector<std::pair<const RenderInterface::Image*, const RenderInterface::ImageView*>>& Attachments,
			const Scene& Scene, bool ResourcesWereRecreated);

		void RecreatePipeline(const RenderInterface::RenderPass& Pass);
	};
}
