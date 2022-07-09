#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "RenderingEngine/FrameGraph/Graph.h"
#include "RenderingEngine/Passes/PBRPass.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"
#include "RenderInterface/GenericRenderInterface/CommonTypes.h"

namespace Hermes
{
	class SkyboxPass;
	class GBufferPass;
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

	private:
		RenderInterface::DeviceProperties GPUProperties;
		GraphicsSettings CurrentSettings;

		std::shared_ptr<RenderInterface::Device> RenderingDevice;
		std::shared_ptr<RenderInterface::Swapchain> Swapchain;

		std::shared_ptr<DescriptorAllocator> DescriptorAllocator;
		
		std::unique_ptr<FrameGraph> FrameGraph;
		std::shared_ptr<GBufferPass> GBufferPass;
		std::unique_ptr<PBRPass> PBRPass;
		std::shared_ptr<SkyboxPass> SkyboxPass;

		static constexpr uint32 NumberOfBackBuffers = 3; // TODO : let user modify
		static constexpr RenderInterface::DataFormat ColorAttachmentFormat = RenderInterface::DataFormat::B8G8R8A8UnsignedNormalized;
		static constexpr RenderInterface::DataFormat DepthAttachmentFormat = RenderInterface::DataFormat::D32SignedFloat;

		void DumpGPUProperties() const;
	};
}
