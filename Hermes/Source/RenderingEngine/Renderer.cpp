#include "Renderer.h"

#include <functional>

#include "ApplicationCore/GameLoop.h"
#include "Core/Profiling.h"
#include "Logging/Logger.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/FrameGraph/Graph.h"
#include "RenderingEngine/SharedData.h"
#include "RenderingEngine/Passes/DepthPass.h"
#include "RenderingEngine/Passes/ForwardPass.h"
#include "RenderingEngine/Passes/LightCullingPass.h"
#include "RenderingEngine/Passes/PostProcessingPass.h"
#include "RenderingEngine/Passes/SkyboxPass.h"
#include "RenderingEngine/Passes/UIPass.h"
#include "RenderingEngine/Scene/Camera.h"
#include "RenderingEngine/Scene/Scene.h"
#include "Vulkan/Device.h"
#include "Vulkan/Fence.h"
#include "Vulkan/Swapchain.h"

namespace Hermes
{
	struct RendererState
	{
		Vulkan::DeviceProperties GPUProperties;

		std::unique_ptr<Vulkan::Instance> VulkanInstance;
		std::unique_ptr<Vulkan::Device> Device;
		std::unique_ptr<Vulkan::Swapchain> Swapchain;

		std::unique_ptr<DescriptorAllocator> DescriptorAllocator;
		std::unique_ptr<Vulkan::DescriptorSetLayout> GlobalDataDescriptorSetLayout;
		std::unique_ptr<Vulkan::Buffer> GlobalSceneDataBuffer;
		std::unique_ptr<Vulkan::Sampler> DefaultSampler;

		ShaderCache ShaderCache;

		std::unique_ptr<FrameGraph> FrameGraph;
		std::unique_ptr<LightCullingPass> LightCullingPass;
		std::unique_ptr<DepthPass> DepthPass;
		std::unique_ptr<ForwardPass> ForwardPass;
		std::unique_ptr<PostProcessingPass> PostProcessingPass;
		std::unique_ptr<SkyboxPass> SkyboxPass;
		std::unique_ptr<UIPass> UIPass;

		Rect2Dui SceneViewport;

		static constexpr uint32 NumberOfBackBuffers = 3; // TODO : let user modify
		static constexpr VkFormat ColorAttachmentFormat = VK_FORMAT_B8G8R8A8_UNORM;
		static constexpr VkFormat DepthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
	};

	RendererState* GRendererState = nullptr;

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

	bool Renderer::Init()
	{
		HERMES_ASSERT_LOG(GRendererState == nullptr, "Trying to initialize the renderer twice");
		GRendererState = new RendererState;

		GRendererState->VulkanInstance = std::make_unique<Vulkan::Instance>(*GGameLoop->GetWindow());
		
		const auto& AvailableGPUs = GRendererState->VulkanInstance->EnumerateAvailableDevices();
		auto GPUIndex = PickGPU(AvailableGPUs);
		GRendererState->GPUProperties = AvailableGPUs[GPUIndex];
		GRendererState->Device = GRendererState->VulkanInstance->CreateDevice(GPUIndex);
		if (!GRendererState->Device)
			return false;
		DumpGPUProperties();

		GRendererState->Swapchain = GRendererState->Device->CreateSwapchain(RendererState::NumberOfBackBuffers);
		if (!GRendererState->Swapchain)
			return false;

		GRendererState->DescriptorAllocator = std::make_unique<DescriptorAllocator>();

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

		GRendererState->GlobalDataDescriptorSetLayout = GRendererState->Device->CreateDescriptorSetLayout({
			SceneUBOBinding, IrradianceCubemapBinding, SpecularCubemapBinding, PrecomputedBRDFBinding, LightClusterListBinding, LightIndexListBinding
		});

		GRendererState->GlobalSceneDataBuffer = GRendererState->Device->CreateBuffer(sizeof(GlobalSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true);

		Vulkan::SamplerDescription SamplerDesc = {};
		SamplerDesc.AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		SamplerDesc.MinificationFilter = VK_FILTER_LINEAR;
		SamplerDesc.MagnificationFilter = VK_FILTER_LINEAR;
		SamplerDesc.CoordinateSystem = Vulkan::CoordinateSystem::Normalized;
		SamplerDesc.MipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		SamplerDesc.MinLOD = 0.0f;
		SamplerDesc.MaxLOD = 13.0f; // 8192x8192 texture is probably enough for everyone :)
		SamplerDesc.LODBias = 0.0f;
		GRendererState->DefaultSampler = GRendererState->Device->CreateSampler(SamplerDesc);

		GRendererState->LightCullingPass = std::make_unique<LightCullingPass>();
		GRendererState->DepthPass = std::make_unique<DepthPass>();
		GRendererState->ForwardPass = std::make_unique<ForwardPass>(true);
		GRendererState->PostProcessingPass = std::make_unique<PostProcessingPass>();
		GRendererState->SkyboxPass = std::make_unique<SkyboxPass>();
		GRendererState->UIPass = std::make_unique<UIPass>();

		FrameGraphScheme Scheme;
		Scheme.AddPass("LightCullingPass", GRendererState->LightCullingPass->GetPassDescription());
		Scheme.AddPass("DepthPass", GRendererState->DepthPass->GetPassDescription());
		Scheme.AddPass("ForwardPass", GRendererState->ForwardPass->GetPassDescription());
		Scheme.AddPass("PostProcessingPass", GRendererState->PostProcessingPass->GetPassDescription());
		Scheme.AddPass("SkyboxPass", GRendererState->SkyboxPass->GetPassDescription());
		Scheme.AddPass("UIPass", GRendererState->UIPass->GetPassDescription());

		ImageResourceDescription HDRColorBufferResource = {};
		HDRColorBufferResource.Dimensions = ViewportRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		HDRColorBufferResource.Format = VK_FORMAT_R16G16B16A16_SFLOAT;
		HDRColorBufferResource.MipLevels = 1;
		Scheme.AddResource("HDRColorBuffer", HDRColorBufferResource);

		ImageResourceDescription ColorBufferResource = {};
		ColorBufferResource.Dimensions = ViewportRelativeDimensions::CreateFromRelativeDimensions({ 1.0f });
		ColorBufferResource.Format = VK_FORMAT_B8G8R8A8_SRGB;
		ColorBufferResource.MipLevels = 1;
		Scheme.AddResource("ColorBuffer", ColorBufferResource);

		ImageResourceDescription DepthBufferResource = {};
		DepthBufferResource.Dimensions = ViewportRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
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

		Scheme.AddLink("PostProcessingPass.OutputColor", "UIPass.Framebuffer");

		Scheme.AddLink("UIPass.Framebuffer", "$.FINAL_IMAGE");

		GRendererState->FrameGraph = Scheme.Compile();
		HERMES_ASSERT_LOG(GRendererState->FrameGraph, "Failed to compile a frame graph");

		return true;
	}

	void Renderer::SetViewport(Rect2Dui NewViewport)
	{
		HERMES_ASSERT(GRendererState);
		if (NewViewport == GRendererState->SceneViewport)
			return;

		HERMES_LOG_INFO("SetSceneViewport(%u, %u, %u, %u)", NewViewport.Min.X, NewViewport.Min.Y, NewViewport.Max.X, NewViewport.Max.Y);
		GRendererState->SceneViewport = NewViewport;
	}

	void Renderer::RunFrame(const Scene& Scene, const UI::Widget* RootWidget)
	{
		HERMES_PROFILE_FUNC();

		HERMES_ASSERT(GRendererState);

		FillSceneDataBuffer(Scene);

		auto GeometryList = Scene.BakeGeometryList();

		GRendererState->UIPass->SetRootWidget(RootWidget);

		auto Metrics = GRendererState->FrameGraph->Execute(Scene, GeometryList, GRendererState->SceneViewport);
		HERMES_PROFILE_TAG("Draw call count", static_cast<int64>(Metrics.DrawCallCount));
		HERMES_PROFILE_TAG("Pipeline bind count", static_cast<int64>(Metrics.PipelineBindCount));
		HERMES_PROFILE_TAG("Descriptor set bind count", static_cast<int64>(Metrics.DescriptorSetBindCount));
		HERMES_PROFILE_TAG("Buffer bind count", static_cast<int64>(Metrics.BufferBindCount));

		auto [FinalImage, FinalImageLayout] = GRendererState->FrameGraph->GetFinalImage();

		Present(*FinalImage, FinalImageLayout, GRendererState->SceneViewport);
	}

	void Renderer::Shutdown()
	{
		HERMES_ASSERT_LOG(GRendererState, "Trying to shut down the renderer twice");
		delete GRendererState;
		GRendererState = nullptr;
	}

	Vulkan::Device& Renderer::GetDevice()
	{
		HERMES_ASSERT(GRendererState);
		return *GRendererState->Device;
	}

	Vec2ui Renderer::GetSwapchainDimensions()
	{
		HERMES_ASSERT(GRendererState);
		return GRendererState->Swapchain->GetDimensions();
	}

	DescriptorAllocator& Renderer::GetDescriptorAllocator()
	{
		HERMES_ASSERT(GRendererState);
		return *GRendererState->DescriptorAllocator;
	}

	ShaderCache& Renderer::GetShaderCache()
	{
		HERMES_ASSERT(GRendererState);
		return GRendererState->ShaderCache;
	}

	const Vulkan::DescriptorSetLayout& Renderer::GetGlobalDataDescriptorSetLayout()
	{
		HERMES_ASSERT(GRendererState);
		return *GRendererState->GlobalDataDescriptorSetLayout;
	}

	const Vulkan::RenderPass& Renderer::GetGraphicsRenderPassObject()
	{
		HERMES_ASSERT(GRendererState);
		return GRendererState->FrameGraph->GetRenderPassObject("ForwardPass");
	}

	const Vulkan::RenderPass& Renderer::GetVertexRenderPassObject()
	{
		HERMES_ASSERT(GRendererState);
		return GRendererState->FrameGraph->GetRenderPassObject("DepthPass");
	}

	const Vulkan::Buffer& Renderer::GetGlobalSceneDataBuffer()
	{
		HERMES_ASSERT(GRendererState);
		return *GRendererState->GlobalSceneDataBuffer;
	}

	const Vulkan::Sampler& Renderer::GetDefaultSampler()
	{
		HERMES_ASSERT(GRendererState);
		return *GRendererState->DefaultSampler;
	}

	void Renderer::DumpGPUProperties()
	{
		HERMES_ASSERT(GRendererState);
		HERMES_LOG_INFO("======== GPU Information ========");
		HERMES_LOG_INFO("Name: %s", GRendererState->GPUProperties.Name.c_str());
		HERMES_LOG_INFO("Anisotropy: %s, %f", GRendererState->GPUProperties.AnisotropySupport ? "true" : "false", GRendererState->GPUProperties.MaxAnisotropyLevel);
	}

	void Renderer::FillSceneDataBuffer(const Scene& Scene)
	{
		HERMES_ASSERT(GRendererState);
		HERMES_PROFILE_FUNC();

		auto& Camera = Scene.GetActiveCamera();

		auto* SceneDataForCurrentFrame = static_cast<GlobalSceneData*>(GRendererState->GlobalSceneDataBuffer->Map());

		auto ViewMatrix = Camera.GetViewMatrix();
		auto ProjectionMatrix = Camera.GetProjectionMatrix();

		SceneDataForCurrentFrame->ViewProjection = ProjectionMatrix * ViewMatrix;
		SceneDataForCurrentFrame->View = ViewMatrix;
		SceneDataForCurrentFrame->InverseProjection = ProjectionMatrix.Inverse();
		SceneDataForCurrentFrame->CameraLocation = { Camera.GetLocation(), 1.0f };

		SceneDataForCurrentFrame->ScreenDimensions = static_cast<Vec2>(GRendererState->Swapchain->GetDimensions());
		SceneDataForCurrentFrame->MaxPixelsPerLightCluster = static_cast<Vec2>(GRendererState->LightCullingPass->ClusterSizeInPixels);
		SceneDataForCurrentFrame->CameraZBounds = { Camera.GetNearZPlane(), Camera.GetFarZPlane() };
		SceneDataForCurrentFrame->NumberOfZClusters.X = GRendererState->LightCullingPass->NumberOfZSlices;

		size_t NextPointLightIndex = 0, NextDirectionalLightIndex = 0;
		std::function<void(const SceneNode&)> TraverseSceneHierarchy = [&](const SceneNode& Node) -> void
		{
			for (size_t ChildIndex = 0; ChildIndex < Node.GetChildrenCount(); ChildIndex++)
				TraverseSceneHierarchy(Node.GetChild(ChildIndex));

			if (Node.GetType() == SceneNodeType::PointLight)
			{
				if (NextPointLightIndex > GlobalSceneData::MaxPointLightCount)
					return;
				
				const auto& PointLight = static_cast<const PointLightNode&>(Node);
				auto WorldTransform = PointLight.GetWorldTransformationMatrix();
				Vec4 WorldPosition = { WorldTransform[0][3], WorldTransform[1][3], WorldTransform[2][3], 1.0f };

				SceneDataForCurrentFrame->PointLights[NextPointLightIndex].Position = WorldPosition;
				SceneDataForCurrentFrame->PointLights[NextPointLightIndex].Color = Vec4(PointLight.GetColor(), PointLight.GetIntensity());

				NextPointLightIndex++;
			}

			if (Node.GetType() == SceneNodeType::DirectionalLight)
			{
				if (NextDirectionalLightIndex > GlobalSceneData::MaxDirectionalLightCount)
					return;

				const auto& DirectionalLight = static_cast<const DirectionalLightNode&>(Node);

				auto WorldTransform = Node.GetWorldTransformationMatrix();
				auto WorldDirection = WorldTransform * Vec4(DirectionalLight.GetDirection(), 0.0f);

				SceneDataForCurrentFrame->DirectionalLights[NextDirectionalLightIndex].Direction = WorldDirection;
				SceneDataForCurrentFrame->DirectionalLights[NextDirectionalLightIndex].Color = Vec4(DirectionalLight.GetColor(), DirectionalLight.GetIntensity());

				NextDirectionalLightIndex++;
			}
		};

		TraverseSceneHierarchy(Scene.GetRootNode());

		if (NextPointLightIndex >= GlobalSceneData::MaxPointLightCount)
			HERMES_LOG_WARNING("There are more point lights in the scene than the shader can process, some of them will be ignored");
		if (NextDirectionalLightIndex >= GlobalSceneData::MaxDirectionalLightCount)
			HERMES_LOG_WARNING("There are more directional lights in the scene than the shader can process, some of them will be ignored");

		SceneDataForCurrentFrame->PointLightCount = static_cast<uint32>(NextPointLightIndex);
		SceneDataForCurrentFrame->DirectionalLightCount = static_cast<uint32>(NextDirectionalLightIndex);
		
		GRendererState->GlobalSceneDataBuffer->Unmap();
	}

	void Renderer::Present(const Vulkan::Image& SourceImage, VkImageLayout CurrentLayout, Rect2Dui Viewport)
	{
		HERMES_ASSERT(GRendererState);
		HERMES_PROFILE_FUNC();

		HERMES_ASSERT(Viewport.Max.X <= GRendererState->Swapchain->GetDimensions().X);
		HERMES_ASSERT(Viewport.Max.Y <= GRendererState->Swapchain->GetDimensions().Y);

		auto& Queue = GRendererState->Device->GetQueue(VK_QUEUE_GRAPHICS_BIT);
		auto CommandBuffer = Queue.CreateCommandBuffer();

		bool Dummy;
		auto SwapchainImageAcquireFence = GRendererState->Device->CreateFence();
		auto MaybeSwapchainImageIndex = GRendererState->Swapchain->AcquireImage(UINT64_MAX, *SwapchainImageAcquireFence, Dummy);
		if (!MaybeSwapchainImageIndex.has_value())
		{
			HERMES_LOG_ERROR("Presentation failed: swapchain did not return valid image index.");
			return;
		}
		const auto& SwapchainImage = GRendererState->Swapchain->GetImage(MaybeSwapchainImageIndex.value());

		CommandBuffer->BeginRecording();

		VkImageMemoryBarrier BarriersBeforeBlit[2];
		// Source image to transfer source optimal
		BarriersBeforeBlit[0] = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
			.oldLayout = CurrentLayout,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.srcQueueFamilyIndex = 0,
			.dstQueueFamilyIndex = 0,
			.image = SourceImage.GetImage(),
			.subresourceRange = SourceImage.GetFullSubresourceRange()
		};
		// Swapchain image to transfer destination optimal
		BarriersBeforeBlit[1] = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = 0,
			.dstQueueFamilyIndex = 0,
			.image = SwapchainImage.GetImage(),
			.subresourceRange = SwapchainImage.GetFullSubresourceRange()
		};
		CommandBuffer->InsertImageMemoryBarriers(BarriersBeforeBlit, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		VkImageBlit BlitRegion = {};
		BlitRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		BlitRegion.srcOffsets[0] = { 0, 0, 0 };
		BlitRegion.srcOffsets[1] = { static_cast<int32>(SourceImage.GetDimensions().X), static_cast<int32>(SourceImage.GetDimensions().Y), 1 };
		BlitRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		BlitRegion.dstOffsets[0] = { static_cast<int32>(Viewport.Min.X), static_cast<int32>(Viewport.Min.Y), 0 };
		BlitRegion.dstOffsets[1] = { static_cast<int32>(Viewport.Max.X), static_cast<int32>(Viewport.Max.Y), 1 };

		CommandBuffer->BlitImage(SourceImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, SwapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { &BlitRegion, 1 }, VK_FILTER_LINEAR);

		// Source image back to its original layout
		VkImageMemoryBarrier SourceImageAfterBlitBarrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
			.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.newLayout = CurrentLayout,
			.srcQueueFamilyIndex = 0,
			.dstQueueFamilyIndex = 0,
			.image = SourceImage.GetImage(),
			.subresourceRange = SourceImage.GetFullSubresourceRange()
		};
		CommandBuffer->InsertImageMemoryBarrier(SourceImageAfterBlitBarrier, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

		// Swapchain image to presentation optimal
		VkImageMemoryBarrier SwapchainImageToPresentationBarrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.srcQueueFamilyIndex = 0,
			.dstQueueFamilyIndex = 0,
			.image = SwapchainImage.GetImage(),
			.subresourceRange = SwapchainImage.GetFullSubresourceRange()
		};
		CommandBuffer->InsertImageMemoryBarrier(SwapchainImageToPresentationBarrier, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		CommandBuffer->EndRecording();

		SwapchainImageAcquireFence->Wait(UINT64_MAX);

		auto BlitFinishedFence = GRendererState->Device->CreateFence();
		Queue.SubmitCommandBuffer(*CommandBuffer, BlitFinishedFence.get());
		BlitFinishedFence->Wait(UINT64_MAX);

		GRendererState->Swapchain->Present(MaybeSwapchainImageIndex.value(), Dummy);
	}
}
