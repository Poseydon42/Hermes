#pragma once

#include <memory>

#include "Core/Core.h"
#include "Math/Math.h"
#include "RenderingEngine/Scene/SceneProxies.h"
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

		std::unique_ptr<RenderInterface::DescriptorSetLayout> SceneUBODescriptorLayout;
		std::unique_ptr<RenderInterface::DescriptorSet> SceneUBODescriptorSet;
		std::unique_ptr<RenderInterface::Buffer> SceneUBOBuffer;

		struct SceneUBO
		{
			Mat4 ViewProjection;
			Vec4 CameraLocation;

			static constexpr size_t MaxPointLightCount = 256;
			PointLightProxy PointLights[MaxPointLightCount];
			uint32 PointLightCount = 0;
		};

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

		void RecreatePipeline(const RenderInterface::RenderPass& Pass, Vec2ui Dimensions);

		/*
		 * Recreates and recomputes the precomputed BRDF image and the corresponding sampler
		 */
		static void EnsurePrecomputedBRDF();
	};
}
