#include "Renderer.h"

#include "Logging/Logger.h"
#include "RenderingEngine/Passes/SkyboxPass.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Scene/Scene.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"
#include "RenderInterface/GenericRenderInterface/CommonTypes.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"

namespace Hermes
{
	Renderer& Renderer::Get()
	{
		static Renderer Instance;
		return Instance;
	}

	bool Renderer::Init(const RenderInterface::PhysicalDevice& GPU)
	{
		RenderingDevice = GPU.CreateDevice();
		if (!RenderingDevice)
			return false;
		GPUProperties = GPU.GetProperties();
		DumpGPUProperties();

		Swapchain = RenderingDevice->CreateSwapchain(NumberOfBackBuffers);
		if (!Swapchain)
			return false;

		DescriptorAllocator = std::make_shared<class DescriptorAllocator>(RenderingDevice);

		ForwardPass = std::make_unique<class ForwardPass>();
		PostProcessingPass = std::make_unique<class PostProcessingPass>();
		SkyboxPass = std::make_unique<class SkyboxPass>(RenderingDevice);

		FrameGraphScheme Scheme;		
		Scheme.AddPass(L"ForwardPass", ForwardPass->GetPassDescription());
		Scheme.AddPass(L"PostProcessingPass", PostProcessingPass->GetPassDescription());
		Scheme.AddPass(L"SkyboxPass", SkyboxPass->GetPassDescription());

		ResourceDesc HDRColorBufferResource = {};
		HDRColorBufferResource.Dimensions = SwapchainRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		HDRColorBufferResource.Format = RenderInterface::DataFormat::R16G16B16A16SignedFloat;
		HDRColorBufferResource.MipLevels = 1;
		Scheme.AddResource(L"HDRColorBuffer", HDRColorBufferResource);

		ResourceDesc ColorBufferResource = {};
		ColorBufferResource.Dimensions = SwapchainRelativeDimensions::CreateFromRelativeDimensions({ 1.0f });
		ColorBufferResource.Format = RenderInterface::DataFormat::B8G8R8A8SRGB;
		ColorBufferResource.MipLevels = 1;
		Scheme.AddResource(L"ColorBuffer", ColorBufferResource);

		ResourceDesc DepthBufferResource = {};
		DepthBufferResource.Dimensions = SwapchainRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		DepthBufferResource.Format = RenderInterface::DataFormat::D32SignedFloat;
		DepthBufferResource.MipLevels = 1;
		Scheme.AddResource(L"DepthBuffer", DepthBufferResource);

		Scheme.AddLink(L"$.HDRColorBuffer", L"ForwardPass.Color");
		Scheme.AddLink(L"$.DepthBuffer", L"ForwardPass.Depth");

		Scheme.AddLink(L"ForwardPass.Color", L"SkyboxPass.ColorBuffer");
		Scheme.AddLink(L"ForwardPass.Depth", L"SkyboxPass.DepthBuffer");

		Scheme.AddLink(L"SkyboxPass.ColorBuffer", L"PostProcessingPass.InputColor");
		Scheme.AddLink(L"$.ColorBuffer", L"PostProcessingPass.OutputColor");

		Scheme.AddLink(L"PostProcessingPass.OutputColor", L"$.BLIT_TO_SWAPCHAIN");

		FrameGraph = Scheme.Compile();

		return true;
	}

	const GraphicsSettings& Renderer::GetGraphicsSettings() const
	{
		return CurrentSettings;
	}

	void Renderer::UpdateGraphicsSettings(GraphicsSettings NewSettings)
	{
		if (NewSettings.AnisotropyLevel != CurrentSettings.AnisotropyLevel)
		{
			Material::SetDefaultAnisotropyLevel(NewSettings.AnisotropyLevel);
		}

		CurrentSettings = NewSettings;
	}

	void Renderer::RunFrame(const Scene& Scene)
	{
		FrameGraph->Execute(Scene);
	}

	RenderInterface::Device& Renderer::GetActiveDevice()
	{
		return *RenderingDevice;
	}

	RenderInterface::Swapchain& Renderer::GetSwapchain()
	{
		return *Swapchain;
	}

	DescriptorAllocator& Renderer::GetDescriptorAllocator()
	{
		return *DescriptorAllocator;
	}

	void Renderer::DumpGPUProperties() const
	{
		HERMES_LOG_INFO(L"======== GPU Information ========");
		HERMES_LOG_INFO(L"Name: %S", GPUProperties.Name.c_str());
		HERMES_LOG_INFO(L"Anisotropy: %s, %f", GPUProperties.AnisotropySupport ? L"true" : L"false", GPUProperties.MaxAnisotropyLevel);
	}
}
