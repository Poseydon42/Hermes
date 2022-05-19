#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderingEngine/FrameGraph/Graph.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"
#include "RenderInterface/GenericRenderInterface/CommonTypes.h"

namespace Hermes
{
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
	public:
		static Renderer& Get();

		bool Init(const RenderInterface::PhysicalDevice& GPU);

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
		
		std::unique_ptr<class FrameGraph> FrameGraph;
		std::shared_ptr<GBufferPass> GBufferPass;

		static constexpr uint32 NumberOfBackBuffers = 3; // TODO : let user modify
		static constexpr RenderInterface::DataFormat ColorAttachmentFormat = RenderInterface::DataFormat::B8G8R8A8UnsignedNormalized;
		static constexpr RenderInterface::DataFormat DepthAttachmentFormat = RenderInterface::DataFormat::D32SignedFloat;

		void DumpGPUProperties() const;
	};
}
