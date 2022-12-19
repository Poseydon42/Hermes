#include "Renderer.h"

#include "ApplicationCore/GameLoop.h"
#include "Core/Profiling.h"
#include "Logging/Logger.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/SharedData.h"
#include "RenderingEngine/Passes/SkyboxPass.h"
#include "RenderingEngine/Scene/Camera.h"
#include "RenderingEngine/Scene/Scene.h"

namespace Hermes
{
	static size_t PickGPU(const std::vector<Vulkan::DeviceProperties>& GPUs)
	{
		for (size_t Index = 0; Index < GPUs.size(); Index++)
		{
			if (GPUs[Index].Type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				return Index;
			}
		}

		return 0;
	}

	Renderer& Renderer::Get()
	{
		static Renderer Instance;
		return Instance;
	}

	bool Renderer::Init()
	{
		VulkanInstance = std::make_unique<Vulkan::Instance>(*GGameLoop->GetWindow());
		
		const auto& AvailableGPUs = VulkanInstance->EnumerateAvailableDevices();
		auto GPUIndex = PickGPU(AvailableGPUs);
		GPUProperties = AvailableGPUs[GPUIndex];
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
		VkDescriptorSetLayoutBinding LightClusterListBinding = {};
		LightClusterListBinding.binding = 4;
		LightClusterListBinding.descriptorCount = 1;
		LightClusterListBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		LightClusterListBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		VkDescriptorSetLayoutBinding LightIndexListBinding = {};
		LightIndexListBinding.binding = 5;
		LightIndexListBinding.descriptorCount = 1;
		LightIndexListBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		LightIndexListBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

		GlobalDataDescriptorSetLayout = Device->CreateDescriptorSetLayout({
			SceneUBOBinding, IrradianceCubemapBinding, SpecularCubemapBinding, PrecomputedBRDFBinding, LightClusterListBinding, LightIndexListBinding
		});

		GlobalSceneDataBuffer = Device->CreateBuffer(sizeof(GlobalSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true);

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

		LightCullingPass = std::make_unique<class LightCullingPass>();
		DepthPass = std::make_unique<class DepthPass>();
		ForwardPass = std::make_unique<class ForwardPass>(true);
		PostProcessingPass = std::make_unique<class PostProcessingPass>();
		SkyboxPass = std::make_unique<class SkyboxPass>();

		FrameGraphScheme Scheme;
		Scheme.AddPass("LightCullingPass", LightCullingPass->GetPassDescription());
		Scheme.AddPass("DepthPass", DepthPass->GetPassDescription());
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

		BufferResourceDescription LightClusterListResource = {};
		// FIXME: find a way to compute this value instead of guessing
		LightClusterListResource.Size = 4 * 1024 * 1024;
		Scheme.AddResource("LightClusterList", LightClusterListResource);

		BufferResourceDescription LightIndexListResource = {};
 		LightIndexListResource.Size = 64 * 1024 * 1024; // FIXME: see above
		Scheme.AddResource("LightIndexList", LightIndexListResource);

		Scheme.AddLink("$.LightClusterList", "LightCullingPass.LightClusterList");
		Scheme.AddLink("$.LightIndexList", "LightCullingPass.LightIndexList");

		Scheme.AddLink("$.DepthBuffer", "DepthPass.Depth");

		Scheme.AddLink("$.HDRColorBuffer", "ForwardPass.Color");
		Scheme.AddLink("DepthPass.Depth", "ForwardPass.Depth");
		Scheme.AddLink("LightCullingPass.LightClusterList", "ForwardPass.LightClusterList");
		Scheme.AddLink("LightCullingPass.LightIndexList", "ForwardPass.LightIndexList");

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

		FillSceneDataBuffer(Scene);

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

	ShaderCache& Renderer::GetShaderCache()
	{
		return ShaderCache;
	}

	const Vulkan::DescriptorSetLayout& Renderer::GetGlobalDataDescriptorSetLayout() const
	{
		return *GlobalDataDescriptorSetLayout;
	}

	const Vulkan::RenderPass& Renderer::GetGraphicsRenderPassObject() const
	{
		return FrameGraph->GetRenderPassObject("ForwardPass");
	}

	const Vulkan::RenderPass& Renderer::GetVertexRenderPassObject() const
	{
		return FrameGraph->GetRenderPassObject("DepthPass");
	}

	const Vulkan::Buffer& Renderer::GetGlobalSceneDataBuffer() const
	{
		return *GlobalSceneDataBuffer;
	}

	const Vulkan::Sampler& Renderer::GetDefaultSampler() const
	{
		return *DefaultSampler;
	}

	void Renderer::DumpGPUProperties() const
	{
		HERMES_LOG_INFO("======== GPU Information ========");
		HERMES_LOG_INFO("Name: %s", GPUProperties.Name.c_str());
		HERMES_LOG_INFO("Anisotropy: %s, %f", GPUProperties.AnisotropySupport ? "true" : "false", GPUProperties.MaxAnisotropyLevel);
	}

	void Renderer::FillSceneDataBuffer(const Scene& Scene)
	{
		HERMES_PROFILE_FUNC();

		auto& Camera = Scene.GetActiveCamera();

		auto* SceneDataForCurrentFrame = static_cast<GlobalSceneData*>(GlobalSceneDataBuffer->Map());

		auto ViewMatrix = Camera.GetViewMatrix();
		auto ProjectionMatrix = Camera.GetProjectionMatrix();

		SceneDataForCurrentFrame->ViewProjection = ProjectionMatrix * ViewMatrix;
		SceneDataForCurrentFrame->View = ViewMatrix;
		SceneDataForCurrentFrame->InverseProjection = ProjectionMatrix.Inverse();
		SceneDataForCurrentFrame->CameraLocation = { Camera.GetLocation(), 1.0f };

		SceneDataForCurrentFrame->ScreenDimensions = static_cast<Vec2>(Swapchain->GetDimensions());
		SceneDataForCurrentFrame->MaxPixelsPerLightCluster = static_cast<Vec2>(LightCullingPass->ClusterSizeInPixels);
		SceneDataForCurrentFrame->CameraZBounds = { Camera.GetNearZPlane(), Camera.GetFarZPlane() };
		SceneDataForCurrentFrame->NumberOfZClusters.X = LightCullingPass->NumberOfZSlices;

		// FIXME: replace assert with log warning
		HERMES_ASSERT_LOG(Scene.GetPointLights().size() < GlobalSceneData::MaxPointLightCount,
		                  "There are more point lights in the scene than the shader can process, some of them will be ignored");
		SceneDataForCurrentFrame->PointLightCount = Math::Min<uint32>(static_cast<uint32>(Scene.GetPointLights().size()),
		                                                             GlobalSceneData::MaxPointLightCount);
		for (size_t Index = 0; Index < Scene.GetPointLights().size(); Index++)
		{
			SceneDataForCurrentFrame->PointLights[Index].Position = Scene.GetPointLights()[Index].Position;
			SceneDataForCurrentFrame->PointLights[Index].Color = Scene.GetPointLights()[Index].Color;
		}

		const auto& DirectionalLights = Scene.GetDirectionalLights();
		HERMES_ASSERT_LOG(DirectionalLights.size() < GlobalSceneData::MaxDirectionalLightCount, "There are more directional lights in the scene than the shader can process");
		SceneDataForCurrentFrame->DirectionalLightCount = static_cast<uint32>(DirectionalLights.size());
		for (size_t Index = 0; Index < DirectionalLights.size(); Index++)
		{
			SceneDataForCurrentFrame->DirectionalLights[Index].Direction = Vec4(DirectionalLights[Index].Direction, 0.0f);
			SceneDataForCurrentFrame->DirectionalLights[Index].Color = Vec4(DirectionalLights[Index].Color, DirectionalLights[Index].Intensity);
		}

		GlobalSceneDataBuffer->Unmap();
	}
}
