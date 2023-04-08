#include "Renderer.h"

#include <functional>

#include "ApplicationCore/GameLoop.h"
#include "Core/Profiling.h"
#include "Logging/Logger.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/SharedData.h"
#include "RenderingEngine/Passes/SkyboxPass.h"
#include "RenderingEngine/Scene/Camera.h"
#include "RenderingEngine/Scene/Scene.h"
#include "Vulkan/Fence.h"

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
		UIPass = std::make_unique<class UIPass>();

		FrameGraphScheme Scheme;
		Scheme.AddPass("LightCullingPass", LightCullingPass->GetPassDescription());
		Scheme.AddPass("DepthPass", DepthPass->GetPassDescription());
		Scheme.AddPass("ForwardPass", ForwardPass->GetPassDescription());
		Scheme.AddPass("PostProcessingPass", PostProcessingPass->GetPassDescription());
		Scheme.AddPass("SkyboxPass", SkyboxPass->GetPassDescription());
		Scheme.AddPass("UIPass", UIPass->GetPassDescription());

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

		FrameGraph = Scheme.Compile();
		HERMES_ASSERT_LOG(FrameGraph, "Failed to compile a frame graph");

		return true;
	}

	void Renderer::SetSceneViewport(Rect2Dui NewViewport)
	{
		if (NewViewport == SceneViewport)
			return;

		HERMES_LOG_INFO("SetSceneViewport(%u, %u, %u, %u)", NewViewport.Min.X, NewViewport.Min.Y, NewViewport.Max.X, NewViewport.Max.Y);
		SceneViewport = NewViewport;
	}

	void Renderer::AddWindow(const UI::Window& Window, Vec2ui ScreenLocation)
	{
		UIPass->AddWindow(&Window, ScreenLocation);
	}

	void Renderer::RunFrame(const Scene& Scene)
	{
		HERMES_PROFILE_FUNC();

		FillSceneDataBuffer(Scene);

		auto GeometryList = Scene.BakeGeometryList();

		auto Metrics = FrameGraph->Execute(Scene, GeometryList, SceneViewport);
		HERMES_PROFILE_TAG("Draw call count", static_cast<int64>(Metrics.DrawCallCount));
		HERMES_PROFILE_TAG("Pipeline bind count", static_cast<int64>(Metrics.PipelineBindCount));
		HERMES_PROFILE_TAG("Descriptor set bind count", static_cast<int64>(Metrics.DescriptorSetBindCount));
		HERMES_PROFILE_TAG("Buffer bind count", static_cast<int64>(Metrics.BufferBindCount));

		auto [FinalImage, FinalImageLayout] = FrameGraph->GetFinalImage();

		Present(*FinalImage, FinalImageLayout, SceneViewport);
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
		
		GlobalSceneDataBuffer->Unmap();
	}

	void Renderer::Present(const Vulkan::Image& SourceImage, VkImageLayout CurrentLayout, Rect2Dui Viewport) const
	{
		HERMES_PROFILE_FUNC();

		HERMES_ASSERT(Viewport.Max.X <= Swapchain->GetDimensions().X);
		HERMES_ASSERT(Viewport.Max.Y <= Swapchain->GetDimensions().Y);

		auto& Queue = Device->GetQueue(VK_QUEUE_GRAPHICS_BIT);
		auto CommandBuffer = Queue.CreateCommandBuffer();

		bool Dummy;
		auto SwapchainImageAcquireFence = Device->CreateFence();
		auto MaybeSwapchainImageIndex = Swapchain->AcquireImage(UINT64_MAX, *SwapchainImageAcquireFence, Dummy);
		if (!MaybeSwapchainImageIndex.has_value())
		{
			HERMES_LOG_ERROR("Presentation failed: swapchain did not return valid image index.");
			return;
		}
		const auto& SwapchainImage = Swapchain->GetImage(MaybeSwapchainImageIndex.value());

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

		auto BlitFinishedFence = Device->CreateFence();
		Queue.SubmitCommandBuffer(*CommandBuffer, BlitFinishedFence.get());
		BlitFinishedFence->Wait(UINT64_MAX);

		Swapchain->Present(MaybeSwapchainImageIndex.value(), Dummy);
	}
}
