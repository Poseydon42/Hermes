#include "Renderer.h"

#include "ApplicationCore/GameLoop.h"
#include "Core/Profiling.h"
#include "Logging/Logger.h"
#include "RenderingEngine/Passes/SkyboxPass.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Scene/Scene.h"

namespace Hermes
{
	Renderer& Renderer::Get()
	{
		static Renderer Instance;
		return Instance;
	}

	bool Renderer::Init()
	{
		VulkanInstance = std::make_unique<Vulkan::Instance>(*GGameLoop->GetWindow());

		constexpr size_t GPUIndex = 0;
		GPUProperties = VulkanInstance->EnumerateAvailableDevices()[GPUIndex];
		Device = VulkanInstance->CreateDevice(GPUIndex);
		if (!Device)
			return false;
		DumpGPUProperties();

		Swapchain = Device->CreateSwapchain(NumberOfBackBuffers);
		if (!Swapchain)
			return false;

		DescriptorAllocator = std::make_unique<class DescriptorAllocator>();

		VkDescriptorSetLayoutBinding SceneUBOBinding = {};
		SceneUBOBinding.binding = 0;
		SceneUBOBinding.descriptorCount = 1;
		SceneUBOBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		SceneUBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		VkDescriptorSetLayoutBinding IrradianceCubemapBinding = {};
		IrradianceCubemapBinding.binding = 1;
		IrradianceCubemapBinding.descriptorCount = 1;
		IrradianceCubemapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		IrradianceCubemapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		VkDescriptorSetLayoutBinding SpecularCubemapBinding = {};
		SpecularCubemapBinding.binding = 2;
		SpecularCubemapBinding.descriptorCount = 1;
		SpecularCubemapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		SpecularCubemapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		VkDescriptorSetLayoutBinding PrecomputedBRDFBinding = {};
		PrecomputedBRDFBinding.binding = 3;
		PrecomputedBRDFBinding.descriptorCount = 1;
		PrecomputedBRDFBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		PrecomputedBRDFBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		GlobalDataDescriptorSetLayout = Device->CreateDescriptorSetLayout({
			SceneUBOBinding, IrradianceCubemapBinding, SpecularCubemapBinding, PrecomputedBRDFBinding
		});

		Vulkan::SamplerDescription SamplerDesc = {};
		SamplerDesc.AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		SamplerDesc.MinificationFilter = VK_FILTER_LINEAR;
		SamplerDesc.MagnificationFilter = VK_FILTER_LINEAR;
		SamplerDesc.CoordinateSystem = Vulkan::CoordinateSystem::Normalized;
		SamplerDesc.MipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		SamplerDesc.MinLOD = 0.0f;
		SamplerDesc.MaxLOD = 13.0f; // 8192x8192 texture is probably enough for everyone :)
		SamplerDesc.LODBias = 0.0f;
		DefaultSampler = Device->CreateSampler(SamplerDesc);

		ForwardPass = std::make_unique<class ForwardPass>();
		PostProcessingPass = std::make_unique<class PostProcessingPass>();
		SkyboxPass = std::make_unique<class SkyboxPass>();

		FrameGraphScheme Scheme;		
		Scheme.AddPass("ForwardPass", ForwardPass->GetPassDescription());
		Scheme.AddPass("PostProcessingPass", PostProcessingPass->GetPassDescription());
		Scheme.AddPass("SkyboxPass", SkyboxPass->GetPassDescription());

		ImageResourceDescription HDRColorBufferResource = {};
		HDRColorBufferResource.Dimensions = SwapchainRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		HDRColorBufferResource.Format = VK_FORMAT_R16G16B16A16_SFLOAT;
		HDRColorBufferResource.MipLevels = 1;
		Scheme.AddResource("HDRColorBuffer", HDRColorBufferResource);

		ImageResourceDescription ColorBufferResource = {};
		ColorBufferResource.Dimensions = SwapchainRelativeDimensions::CreateFromRelativeDimensions({ 1.0f });
		ColorBufferResource.Format = VK_FORMAT_B8G8R8A8_SRGB;
		ColorBufferResource.MipLevels = 1;
		Scheme.AddResource("ColorBuffer", ColorBufferResource);

		ImageResourceDescription DepthBufferResource = {};
		DepthBufferResource.Dimensions = SwapchainRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		DepthBufferResource.Format = VK_FORMAT_D32_SFLOAT;
		DepthBufferResource.MipLevels = 1;
		Scheme.AddResource("DepthBuffer", DepthBufferResource);

		Scheme.AddLink("$.HDRColorBuffer", "ForwardPass.Color");
		Scheme.AddLink("$.DepthBuffer", "ForwardPass.Depth");

		Scheme.AddLink("ForwardPass.Color", "SkyboxPass.ColorBuffer");
		Scheme.AddLink("ForwardPass.Depth", "SkyboxPass.DepthBuffer");

		Scheme.AddLink("SkyboxPass.ColorBuffer", "PostProcessingPass.InputColor");
		Scheme.AddLink("$.ColorBuffer", "PostProcessingPass.OutputColor");

		Scheme.AddLink("PostProcessingPass.OutputColor", "$.BLIT_TO_SWAPCHAIN");

		FrameGraph = Scheme.Compile();
		HERMES_ASSERT_LOG(FrameGraph, "Failed to compile a frame graph");

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
		HERMES_PROFILE_FUNC();
		auto GeometryList = Scene.BakeGeometryList();
		auto Metrics = FrameGraph->Execute(Scene, GeometryList);
		HERMES_PROFILE_TAG("Draw call count", static_cast<int64>(Metrics.DrawCallCount));
		HERMES_PROFILE_TAG("Pipeline bind count", static_cast<int64>(Metrics.PipelineBindCount));
		HERMES_PROFILE_TAG("Descriptor set bind count", static_cast<int64>(Metrics.DescriptorSetBindCount));
		HERMES_PROFILE_TAG("Buffer bind count", static_cast<int64>(Metrics.BufferBindCount));
	}

	Vulkan::Device& Renderer::GetActiveDevice()
	{
		return *Device;
	}

	Vulkan::Swapchain& Renderer::GetSwapchain()
	{
		return *Swapchain;
	}

	DescriptorAllocator& Renderer::GetDescriptorAllocator()
	{
		return *DescriptorAllocator;
	}

	const Vulkan::DescriptorSetLayout& Renderer::GetGlobalDataDescriptorSetLayout() const
	{
		return *GlobalDataDescriptorSetLayout;
	}

	const Vulkan::RenderPass& Renderer::GetGraphicsRenderPassObject() const
	{
		return FrameGraph->GetRenderPassObject("ForwardPass");
	}

	const Vulkan::Sampler& Renderer::GetDefaultSampler() const
	{
		return *DefaultSampler;
	}

	void Renderer::DumpGPUProperties() const
	{
		HERMES_LOG_INFO("======== GPU Information ========");
		HERMES_LOG_INFO("Name: %S", GPUProperties.Name.c_str());
		HERMES_LOG_INFO("Anisotropy: %s, %f", GPUProperties.AnisotropySupport ? "true" : "false", GPUProperties.MaxAnisotropyLevel);
	}
}
