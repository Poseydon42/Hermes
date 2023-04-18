#pragma once

#include "Core/Core.h"
#include "RenderingEngine/FrameGraph/Graph.h"
#include "RenderingEngine/Passes/DepthPass.h"
#include "RenderingEngine/Passes/ForwardPass.h"
#include "RenderingEngine/Passes/LightCullingPass.h"
#include "RenderingEngine/Passes/PostProcessingPass.h"
#include "RenderingEngine/Passes/SkyboxPass.h"
#include "Vulkan/Buffer.h"

namespace Hermes
{
	class HERMES_API SceneRenderer
	{
	public:
		SceneRenderer();

		std::pair<const Vulkan::Image*, VkImageLayout> Render(const Scene& Scene, Vec2ui ViewportDimensions) const;

	private:
		std::unique_ptr<Vulkan::Buffer> SceneDataBuffer;

		std::unique_ptr<FrameGraph> FrameGraph;
		std::unique_ptr<LightCullingPass> LightCullingPass;
		std::unique_ptr<DepthPass> DepthPass;
		std::unique_ptr<ForwardPass> ForwardPass;
		std::unique_ptr<PostProcessingPass> PostProcessingPass;
		std::unique_ptr<SkyboxPass> SkyboxPass;

		void UpdateSceneDataBuffer(const Scene& Scene, Vec2ui ViewportDimensions) const;
	};
}
