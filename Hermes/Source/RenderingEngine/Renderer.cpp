#include "Renderer.h"

#include <optick.h>

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

		RenderInterface::DescriptorBinding SceneUBOBinding = {};
		SceneUBOBinding.Index = 0;
		SceneUBOBinding.DescriptorCount = 1;
		SceneUBOBinding.Shader = RenderInterface::ShaderType::VertexShader |
			RenderInterface::ShaderType::FragmentShader;
		SceneUBOBinding.Type = RenderInterface::DescriptorType::UniformBuffer;
		RenderInterface::DescriptorBinding IrradianceCubemapBinding = {};
		IrradianceCubemapBinding.Index = 1;
		IrradianceCubemapBinding.DescriptorCount = 1;
		IrradianceCubemapBinding.Shader = RenderInterface::ShaderType::FragmentShader;
		IrradianceCubemapBinding.Type = RenderInterface::DescriptorType::CombinedSampler;
		RenderInterface::DescriptorBinding SpecularCubemapBinding = {};
		SpecularCubemapBinding.Index = 2;
		SpecularCubemapBinding.DescriptorCount = 1;
		SpecularCubemapBinding.Shader = RenderInterface::ShaderType::FragmentShader;
		SpecularCubemapBinding.Type = RenderInterface::DescriptorType::CombinedSampler;
		RenderInterface::DescriptorBinding PrecomputedBRDFBinding = {};
		PrecomputedBRDFBinding.Index = 3;
		PrecomputedBRDFBinding.DescriptorCount = 1;
		PrecomputedBRDFBinding.Shader = RenderInterface::ShaderType::FragmentShader;
		PrecomputedBRDFBinding.Type = RenderInterface::DescriptorType::CombinedSampler;

		GlobalDataDescriptorSetLayout = RenderingDevice->CreateDescriptorSetLayout({
			SceneUBOBinding, IrradianceCubemapBinding, SpecularCubemapBinding, PrecomputedBRDFBinding
		});

		RenderInterface::SamplerDescription SamplerDesc = {};
		SamplerDesc.AddressingModeU = RenderInterface::AddressingMode::Repeat;
		SamplerDesc.AddressingModeV = RenderInterface::AddressingMode::Repeat;
		SamplerDesc.MinificationFilteringMode = RenderInterface::FilteringMode::Linear;
		SamplerDesc.MagnificationFilteringMode = RenderInterface::FilteringMode::Linear;
		SamplerDesc.CoordinateSystem = RenderInterface::CoordinateSystem::Normalized;
		SamplerDesc.MipMode = RenderInterface::MipmappingMode::Linear;
		SamplerDesc.MinMipLevel = 0.0f;
		SamplerDesc.MaxMipLevel = 13.0f; // 8192x8192 texture is probably enough for everyone :)
		SamplerDesc.MipBias = 0.0f;
		DefaultSampler = RenderingDevice->CreateSampler(SamplerDesc);

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
		HERMES_ASSERT_LOG(FrameGraph, L"Failed to compile a frame graph");

		return true;
	}

	const GraphicsSettings& Renderer::GetGraphicsSettings() const
	{
		return CurrentSettings;
	}

	void Renderer::UpdateGraphicsSettings(GraphicsSettings NewSettings)
	{
		CurrentSettings = NewSettings;
	}

	void Renderer::RunFrame(const Scene& Scene)
	{
		OPTICK_EVENT();
		auto Metrics = FrameGraph->Execute(Scene);
		OPTICK_TAG("Drawm call count", Metrics.DrawCallCount);
		OPTICK_TAG("Pipeline bind count", Metrics.PipelineBindCount);
		OPTICK_TAG("Buffer bind count", Metrics.BufferBindCount);
		OPTICK_TAG("Descriptor set bind count", Metrics.DescriptorSetBindCount);
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

	const RenderInterface::DescriptorSetLayout& Renderer::GetGlobalDataDescriptorSetLayout() const
	{
		return *GlobalDataDescriptorSetLayout;
	}

	const RenderInterface::RenderPass& Renderer::GetGraphicsRenderPassObject() const
	{
		return FrameGraph->GetRenderPassObject(L"ForwardPass");
	}

	const RenderInterface::Sampler& Renderer::GetDefaultSampler() const
	{
		return *DefaultSampler;
	}

	void Renderer::DumpGPUProperties() const
	{
		HERMES_LOG_INFO(L"======== GPU Information ========");
		HERMES_LOG_INFO(L"Name: %S", GPUProperties.Name.c_str());
		HERMES_LOG_INFO(L"Anisotropy: %s, %f", GPUProperties.AnisotropySupport ? L"true" : L"false", GPUProperties.MaxAnisotropyLevel);
	}
}
