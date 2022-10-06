#pragma once

#include <memory>

#include "Core/Core.h"
#include "Math/Math.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "RenderInterface/GenericRenderInterface/Buffer.h"
#include "RenderInterface/GenericRenderInterface/Image.h"
#include "RenderInterface/GenericRenderInterface/Pipeline.h"
#include "RenderInterface/GenericRenderInterface/Sampler.h"
#include "RenderInterface/GenericRenderInterface/Shader.h"

namespace Hermes
{
	class HERMES_API ForwardPass
	{
	public:
		ForwardPass();

		const PassDesc& GetPassDescription() const;

	private:
		
		std::unique_ptr<RenderInterface::DescriptorSet> SceneUBODescriptorSet;
		std::unique_ptr<RenderInterface::Buffer> SceneUBOBuffer;

		bool PipelineWasCreated = false;

		std::unique_ptr<RenderInterface::Pipeline> Pipeline;
		std::unique_ptr<RenderInterface::Sampler> EnvmapSampler;
		std::unique_ptr<RenderInterface::Shader> VertexShader, FragmentShader;

		static std::unique_ptr<RenderInterface::Image> PrecomputedBRDFImage;
		static std::unique_ptr<RenderInterface::ImageView> PrecomputedBRDFView;
		static std::unique_ptr<RenderInterface::Sampler> PrecomputedBRDFSampler;

		PassDesc Description;

		void PassCallback(RenderInterface::CommandBuffer& CommandBuffer,
		                  const RenderInterface::RenderPass& PassInstance,
		                  const std::vector<std::pair<const RenderInterface::Image*, const RenderInterface::ImageView*>>
		                  & Attachments, const Scene& Scene, bool ResourcesWereRecreated);

		/*
		 * Recreates and recomputes the precomputed BRDF image and the corresponding sampler
		 */
		static void EnsurePrecomputedBRDF();
	};
}
