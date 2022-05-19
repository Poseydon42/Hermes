#include "Renderer.h"

#include "RenderingEngine/Passes/GraphicsPass.h"
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

		FrameGraphScheme Scheme;		
		Scheme.AddPass(L"GraphicsPass", GBufferPass->GetPassDescription());

		ResourceDesc BackbufferResource = {};
		BackbufferResource.Dimensions = SwapchainRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		BackbufferResource.Format = RenderInterface::DataFormat::B8G8R8A8UnsignedNormalized;
		BackbufferResource.MipLevels = 1;
		Scheme.AddResource(L"Backbuffer", BackbufferResource);

		ResourceDesc DepthBufferResource = {};
		DepthBufferResource.Format = RenderInterface::DataFormat::D24UnsignedNormalizedS8UnsignedInteger;
		DepthBufferResource.Dimensions = SwapchainRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		DepthBufferResource.MipLevels = 1;
		Scheme.AddResource(L"DepthBuffer", DepthBufferResource);

		Scheme.AddLink(L"$.Backbuffer", L"GraphicsPass.GBuffer");
		Scheme.AddLink(L"$.DepthBuffer", L"GraphicsPass.DepthBuffer");
		Scheme.AddLink(L"GraphicsPass.GBuffer", L"$.BLIT_TO_SWAPCHAIN");

		FrameGraph = Scheme.Compile();

		return true;
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
