#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "Vulkan/Descriptor.h"
#include "Vulkan/Sampler.h"

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
		
		std::unique_ptr<Vulkan::Pipeline> Pipeline;
		std::unique_ptr<Vulkan::Sampler> InputColorSampler;

		PassDesc Description;

		void PassCallback(const PassCallbackInfo& CallbackInfo);

		void CreatePipeline(VkFormat ColorAttachmentFormat);
	};
}
