#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "RenderingEngine/FrameGraph/Graph.h"
#include "RenderingEngine/Passes/ForwardPass.h"
#include "RenderingEngine/Passes/PostProcessingPass.h"
#include "RenderingEngine/Passes/SkyboxPass.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"
#include "RenderInterface/GenericRenderInterface/CommonTypes.h"

namespace Hermes
{
	class DescriptorAllocator;
	class Scene;
	class IPlatformWindow;

	struct GraphicsSettings
	{
		float AnisotropyLevel = 1.0;
	};

	class HERMES_API Renderer
	{
		MAKE_NON_COPYABLE(Renderer)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(Renderer)
		ADD_DEFAULT_DESTRUCTOR(Renderer)
		ADD_DEFAULT_CONSTRUCTOR(Renderer)

	public:
		static Renderer& Get();

		bool Init(const RenderInterface::PhysicalDevice& GPU);

		const GraphicsSettings& GetGraphicsSettings() const;

		void UpdateGraphicsSettings(GraphicsSettings NewSettings);

		void RunFrame(const Scene& Scene);

		RenderInterface::Device& GetActiveDevice();

		RenderInterface::Swapchain& GetSwapchain();

		DescriptorAllocator& GetDescriptorAllocator();

		const RenderInterface::DescriptorSetLayout& GetGlobalDataDescriptorSetLayout() const;

		const RenderInterface::RenderPass& GetGraphicsRenderPassObject() const;

		const RenderInterface::Sampler& GetDefaultSampler() const;

	private:
		RenderInterface::DeviceProperties GPUProperties;
		GraphicsSettings CurrentSettings;

		std::shared_ptr<RenderInterface::Device> RenderingDevice;
		std::shared_ptr<RenderInterface::Swapchain> Swapchain;

		std::shared_ptr<DescriptorAllocator> DescriptorAllocator;
		std::unique_ptr<RenderInterface::DescriptorSetLayout> GlobalDataDescriptorSetLayout;
		std::unique_ptr<RenderInterface::Sampler> DefaultSampler;
		
		std::unique_ptr<FrameGraph> FrameGraph;
		std::unique_ptr<ForwardPass> ForwardPass;
		std::unique_ptr<PostProcessingPass> PostProcessingPass;
		std::unique_ptr<SkyboxPass> SkyboxPass;

		static constexpr uint32 NumberOfBackBuffers = 3; // TODO : let user modify
		static constexpr RenderInterface::DataFormat ColorAttachmentFormat = RenderInterface::DataFormat::B8G8R8A8UnsignedNormalized;
		static constexpr RenderInterface::DataFormat DepthAttachmentFormat = RenderInterface::DataFormat::D32SignedFloat;

		void DumpGPUProperties() const;
	};
}
