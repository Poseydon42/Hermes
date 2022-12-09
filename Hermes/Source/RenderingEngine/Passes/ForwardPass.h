#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Image.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Sampler.h"

namespace Hermes
{
	struct FrameMetrics;

	class HERMES_API ForwardPass
	{
	public:
		ForwardPass(bool ReuseDataInDepthBuffer);

		const PassDesc& GetPassDescription() const;

	private:
		std::unique_ptr<Vulkan::DescriptorSet> SceneUBODescriptorSet;

		std::unique_ptr<Vulkan::Sampler> EnvmapSampler;

		static std::unique_ptr<Vulkan::Image> PrecomputedBRDFImage;
		static std::unique_ptr<Vulkan::ImageView> PrecomputedBRDFView;
		static std::unique_ptr<Vulkan::Sampler> PrecomputedBRDFSampler;

		PassDesc Description;

		void PassCallback(const PassCallbackInfo& CallbackInfo);

		/*
		 * Recreates and recomputes the precomputed BRDF image and the corresponding sampler
		 */
		static void EnsurePrecomputedBRDF();
	};
}
