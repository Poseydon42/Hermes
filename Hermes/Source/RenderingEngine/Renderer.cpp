#include "Renderer.h"

#include <functional>

#include "ApplicationCore/GameLoop.h"
#include "Core/Profiling.h"
#include "Logging/Logger.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/SceneRenderer.h"
#include "RenderingEngine/Scene/Scene.h"
#include "RenderingEngine/UIRenderer.h"
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
		std::unique_ptr<Vulkan::Sampler> DefaultSampler;

		std::unique_ptr<SceneRenderer> SceneRenderer;
		std::unique_ptr<UIRenderer> UIRenderer;

		ShaderCache ShaderCache;

		static constexpr uint32 NumberOfBackBuffers = 3; // TODO : let user modify
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

		GRendererState->SceneRenderer = std::make_unique<SceneRenderer>();
		GRendererState->UIRenderer = std::make_unique<UIRenderer>();

		return true;
	}

	void Renderer::RunFrame(const Scene& Scene, const UI::Widget& RootWidget)
	{
		HERMES_PROFILE_FUNC();

		HERMES_ASSERT(GRendererState);

		auto SceneViewport = GRendererState->UIRenderer->PrepareToRender(RootWidget, GetSwapchainDimensions());

		auto [SceneImage, SceneImageLayout] = GRendererState->SceneRenderer->Render(Scene, SceneViewport.Dimensions());
		HERMES_ASSERT(SceneImage);

		auto [FinalImage, FinalImageLayout] = GRendererState->UIRenderer->Render(*SceneImage, SceneImageLayout);
		HERMES_ASSERT(FinalImage);

		Present(*FinalImage, FinalImageLayout, { { 0, 0 }, GetSwapchainDimensions() });

		HERMES_PROFILE_TAG("Draw call count", static_cast<int64>(Vulkan::GProfilingMetrics.DrawCallCount));
		HERMES_PROFILE_TAG("Compute dispatch count", static_cast<int64>(Vulkan::GProfilingMetrics.ComputeDispatchCount));
		HERMES_PROFILE_TAG("Pipeline bind count", static_cast<int64>(Vulkan::GProfilingMetrics.PipelineBindCount));
		HERMES_PROFILE_TAG("Descriptor set bind count", static_cast<int64>(Vulkan::GProfilingMetrics.DescriptorSetBindCount));
		HERMES_PROFILE_TAG("Buffer bind count", static_cast<int64>(Vulkan::GProfilingMetrics.BufferBindCount));
		Vulkan::GProfilingMetrics = {};
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
