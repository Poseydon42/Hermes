#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "Vulkan/Forward.h"
#include "Vulkan/Shader.h"

namespace Hermes
{
	class CubemapTexture;
	struct FrameMetrics;
	class ImageAsset;
	struct PassDesc;
	class Scene;
	class Texture;

	class HERMES_API SkyboxPass
	{
	public:
		SkyboxPass();

		const PassDesc& GetPassDescription() const;

	private:
		std::unique_ptr<Vulkan::DescriptorSetLayout> DataDescriptorLayout;
		std::unique_ptr<Vulkan::DescriptorSet> DataDescriptorSet;
		std::unique_ptr<Vulkan::Sampler> EnvmapSampler;

		std::unique_ptr<Vulkan::Shader> VertexShader, FragmentShader;
		std::unique_ptr<Vulkan::Pipeline> Pipeline;
		bool IsPipelineCreated = false;

		PassDesc Description;

		void PassCallback(Vulkan::CommandBuffer& CommandBuffer,
		                  const Vulkan::RenderPass& PassInstance,
		                  const std::vector<std::pair<const Vulkan::Image*, const Vulkan::ImageView*>>&,
		                  const Scene& Scene, const GeometryList& GeometryList, FrameMetrics& Metrics, bool ResourcesWereRecreated);

		void RecreatePipeline(const Vulkan::RenderPass& Pass);
	};
}
