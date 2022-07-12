#include "Renderer.h"

#include "RenderingEngine/Passes/PBRPass.h"
#include "RenderingEngine/Passes/GBufferPass.h"
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

		GBufferPass = std::make_shared<class GBufferPass>(RenderingDevice);
		PBRPass = std::make_unique<class PBRPass>();
		SkyboxPass = std::make_shared<class SkyboxPass>(RenderingDevice);
		PostProcessingPass = std::make_unique<class PostProcessingPass>();

		FrameGraphScheme Scheme;		
		Scheme.AddPass(L"GBufferPass", GBufferPass->GetPassDescription());
		Scheme.AddPass(L"PBRPass", PBRPass->GetPassDescription());
		Scheme.AddPass(L"SkyboxPass", SkyboxPass->GetPassDescription());
		Scheme.AddPass(L"PostProcessingPass", PostProcessingPass->GetPassDescription());

		ResourceDesc HDRColorBufferResource = {};
		HDRColorBufferResource.Dimensions = SwapchainRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		HDRColorBufferResource.Format = RenderInterface::DataFormat::R16G16B16A16SignedFloat;
		HDRColorBufferResource.MipLevels = 1;
		Scheme.AddResource(L"HDRColorBuffer", HDRColorBufferResource);

		ResourceDesc ColorBufferResource = {};
		ColorBufferResource.Dimensions = SwapchainRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		ColorBufferResource.Format = RenderInterface::DataFormat::B8G8R8A8SRGB;
		ColorBufferResource.MipLevels = 1;
		Scheme.AddResource(L"ColorBuffer", ColorBufferResource);

		ResourceDesc AlbedoResource = {};
		AlbedoResource.Dimensions = SwapchainRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		AlbedoResource.Format = RenderInterface::DataFormat::R16G16B16A16SignedFloat;
		AlbedoResource.MipLevels = 1;
		Scheme.AddResource(L"Albedo", AlbedoResource);

		ResourceDesc PositionRoughnessResource = {};
		PositionRoughnessResource.Dimensions = SwapchainRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		PositionRoughnessResource.Format = RenderInterface::DataFormat::R16G16B16A16SignedFloat;
		PositionRoughnessResource.MipLevels = 1;
		Scheme.AddResource(L"PositionRoughness", PositionRoughnessResource);

		ResourceDesc NormalMetallicResource = {};
		NormalMetallicResource.Dimensions = SwapchainRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		NormalMetallicResource.Format = RenderInterface::DataFormat::R16G16B16A16SignedFloat;
		NormalMetallicResource.MipLevels = 1;
		Scheme.AddResource(L"NormalMetallic", NormalMetallicResource);

		ResourceDesc DepthBufferResource = {};
		DepthBufferResource.Dimensions = SwapchainRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		DepthBufferResource.Format = RenderInterface::DataFormat::D32SignedFloat;
		DepthBufferResource.MipLevels = 1;
		Scheme.AddResource(L"DepthBuffer", DepthBufferResource);

		Scheme.AddLink(L"$.Albedo", L"GBufferPass.Albedo");
		Scheme.AddLink(L"$.PositionRoughness", L"GBufferPass.PositionRoughness");
		Scheme.AddLink(L"$.NormalMetallic", L"GBufferPass.NormalMetallic");
		Scheme.AddLink(L"$.DepthBuffer", L"GBufferPass.Depth");

		Scheme.AddLink(L"GBufferPass.Albedo", L"PBRPass.Albedo");
		Scheme.AddLink(L"GBufferPass.PositionRoughness", L"PBRPass.PositionRoughness");
		Scheme.AddLink(L"GBufferPass.NormalMetallic", L"PBRPass.NormalMetallic");
		Scheme.AddLink(L"$.HDRColorBuffer", L"PBRPass.ColorBuffer");

		Scheme.AddLink(L"GBufferPass.Depth", L"SkyboxPass.DepthBuffer");
		Scheme.AddLink(L"PBRPass.ColorBuffer", L"SkyboxPass.ColorBuffer");

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
