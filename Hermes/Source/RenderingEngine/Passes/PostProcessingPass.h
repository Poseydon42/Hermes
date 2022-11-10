#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "Vulkan/Descriptor.h"

namespace Hermes
{
	struct FrameMetrics;

	class HERMES_API PostProcessingPass
	{
		MAKE_NON_COPYABLE(PostProcessingPass)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(PostProcessingPass)
		ADD_DEFAULT_DESTRUCTOR(PostProcessingPass)

	public:
		PostProcessingPass();

		const PassDesc& GetPassDescription() const;

	private:
		std::shared_ptr<Vulkan::DescriptorSetLayout> DescriptorLayout;
		std::unique_ptr<Vulkan::DescriptorSet> DescriptorSet;

		bool IsPipelineCreated = false;
		std::unique_ptr<Vulkan::Pipeline> Pipeline;
		std::shared_ptr<Vulkan::Shader> VertexShader, FragmentShader;

		PassDesc Description;

		void PassCallback(const PassCallbackInfo& CallbackInfo);

		void RecreatePipeline(const Vulkan::RenderPass& Pass);
	};
}
