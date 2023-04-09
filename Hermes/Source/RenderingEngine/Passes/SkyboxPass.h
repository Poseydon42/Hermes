#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "Vulkan/Forward.h"
#include "Vulkan/Shader.h"

namespace Hermes
{
	struct FrameMetrics;
	class ImageAsset;
	struct PassDesc;
	class Scene;

	class HERMES_API SkyboxPass
	{
	public:
		SkyboxPass();

		const PassDesc& GetPassDescription() const;

	private:
		std::unique_ptr<Vulkan::DescriptorSetLayout> DataDescriptorLayout;
		std::unique_ptr<Vulkan::DescriptorSet> DataDescriptorSet;
		std::unique_ptr<Vulkan::Sampler> EnvmapSampler;
		
		std::unique_ptr<Vulkan::Pipeline> Pipeline;

		PassDesc Description;

		void PassCallback(const PassCallbackInfo& CallbackInfo);

		void CreatePipeline(VkFormat ColorAttachmentFormat, VkFormat DepthAttachmentFormat);
	};
}
