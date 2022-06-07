#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"

namespace Hermes
{
	class ImageAsset;
	struct PassDesc;
	class Scene;
	class Texture;

	class HERMES_API SkyboxPass
	{
	public:
		SkyboxPass(std::shared_ptr<RenderInterface::Device> InDevice);

		const PassDesc& GetPassDescription() const;
	private:
		struct SkyboxPassData
		{
			float Pitch = 0.0f;
			float Yaw = 0.0f;
			float VerticalFOV = 0.0f;
			float HorizontalFOV = 0.0f;
		};

		std::shared_ptr<RenderInterface::Device> Device;

		std::shared_ptr<RenderInterface::DescriptorSetLayout> DataDescriptorLayout;
		std::shared_ptr<RenderInterface::DescriptorSet> DataDescriptorSet;

		std::shared_ptr<RenderInterface::Buffer> UniformBuffer;
		std::shared_ptr<ImageAsset> EnvmapAsset;
		std::shared_ptr<Texture> EnvmapTexture;
		std::shared_ptr<RenderInterface::Sampler> EnvmapSampler;

		std::shared_ptr<RenderInterface::Shader> VertexShader, FragmentShader;
		std::shared_ptr<RenderInterface::Pipeline> Pipeline;
		bool IsPipelineCreated = false;

		std::shared_ptr<Texture> SkyboxTexture;

		PassDesc Description;

		void PassCallback(
			RenderInterface::CommandBuffer& CommandBuffer,
			const RenderInterface::RenderPass& PassInstance,
			const std::vector<const RenderInterface::Image*>&,
			const Scene& Scene, bool ResourcesWereRecreated);

		void RecreatePipeline(const RenderInterface::RenderPass& Pass);
	};
}
