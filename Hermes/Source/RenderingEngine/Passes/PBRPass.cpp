#include "PBRPass.h"

#include "Logging/Logger.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Scene/Camera.h"
#include "RenderingEngine/Scene/Scene.h"
#include "RenderInterface/GenericRenderInterface/Buffer.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/Fence.h"
#include "RenderInterface/GenericRenderInterface/Pipeline.h"
#include "RenderInterface/GenericRenderInterface/Queue.h"
#include "RenderInterface/GenericRenderInterface/Shader.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"

namespace Hermes
{
	std::unique_ptr<RenderInterface::Image> PBRPass::PrecomputedBRDF;
	std::unique_ptr<RenderInterface::ImageView> PBRPass::PrecomputedBRDFView;
	std::unique_ptr<RenderInterface::Sampler> PBRPass::PrecomputedBRDFSampler;

	PBRPass::PBRPass()
	{
		auto& Device = Renderer::Get().GetActiveDevice();
		auto& DescriptorAllocator = Renderer::Get().GetDescriptorAllocator();

		RenderInterface::DescriptorBinding AlbedoAttachmentBinding = {
			0, 1, RenderInterface::ShaderType::FragmentShader, RenderInterface::DescriptorType::InputAttachment
		};
		RenderInterface::DescriptorBinding PositionRoughnessBinding = {
			1, 1, RenderInterface::ShaderType::FragmentShader, RenderInterface::DescriptorType::InputAttachment
		};
		RenderInterface::DescriptorBinding NormalMetallicBinding = {
			2, 1, RenderInterface::ShaderType::FragmentShader, RenderInterface::DescriptorType::InputAttachment
		};
		RenderInterface::DescriptorBinding UBOBinding = {
			3, 1, RenderInterface::ShaderType::FragmentShader, RenderInterface::DescriptorType::UniformBuffer
		};
		RenderInterface::DescriptorBinding IrradianceMapBinding = {
			4, 1, RenderInterface::ShaderType::FragmentShader, RenderInterface::DescriptorType::CombinedSampler
		};
		RenderInterface::DescriptorBinding SpecularMapBinding = {
			5, 1, RenderInterface::ShaderType::FragmentShader, RenderInterface::DescriptorType::CombinedSampler
		};
		RenderInterface::DescriptorBinding PrecomputedBRDFMapBinding = {
			6, 1, RenderInterface::ShaderType::FragmentShader, RenderInterface::DescriptorType::CombinedSampler
		};
		DescriptorLayout = Device.CreateDescriptorSetLayout({
			                                                    AlbedoAttachmentBinding, PositionRoughnessBinding,
			                                                    NormalMetallicBinding, UBOBinding, IrradianceMapBinding,
			                                                    SpecularMapBinding, PrecomputedBRDFMapBinding
		                                                    });
		DescriptorSet = DescriptorAllocator.Allocate(*DescriptorLayout);

		LightingDataUniformBuffer = Device.CreateBuffer(sizeof(LightingData),
		                                                RenderInterface::BufferUsageType::UniformBuffer |
		                                                RenderInterface::BufferUsageType::CPUAccessible);
		DescriptorSet->UpdateWithBuffer(3, 0, *LightingDataUniformBuffer, 0, sizeof(LightingData));

		VertexShader = Device.CreateShader(L"Shaders/Bin/fs_vert.glsl.spv",
		                                   RenderInterface::ShaderType::VertexShader);
		FragmentShader = Device.CreateShader(L"Shaders/Bin/fs_pbr_frag.glsl.spv",
		                                   RenderInterface::ShaderType::FragmentShader);

		RenderInterface::SamplerDescription SamplerDescription = {};
		SamplerDescription.AddressingModeU = SamplerDescription.AddressingModeV =
			RenderInterface::AddressingMode::Repeat;
		SamplerDescription.MinificationFilteringMode = RenderInterface::FilteringMode::Linear;
		SamplerDescription.MagnificationFilteringMode = RenderInterface::FilteringMode::Linear;
		SamplerDescription.CoordinateSystem = RenderInterface::CoordinateSystem::Normalized;
		SamplerDescription.MipMode = RenderInterface::MipmappingMode::Linear;
		SamplerDescription.MinMipLevel = 0.0f;
		SamplerDescription.MaxMipLevel = 12.0f; // NOTE : up to 2^12 (4096) pixels, should be more than enough
		EnvmapSampler = Device.CreateSampler(SamplerDescription);

		Drain AlbedoDrain = {};
		AlbedoDrain.Name = L"Albedo";
		AlbedoDrain.Binding = BindingMode::InputAttachment;
		AlbedoDrain.Layout = RenderInterface::ImageLayout::ShaderReadOnlyOptimal;
		AlbedoDrain.LoadOp = RenderInterface::AttachmentLoadOp::Load;
		AlbedoDrain.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;

		Drain PositionRoughnessDrain = {};
		PositionRoughnessDrain.Name = L"PositionRoughness";
		PositionRoughnessDrain.Binding = BindingMode::InputAttachment;
		PositionRoughnessDrain.Layout = RenderInterface::ImageLayout::ShaderReadOnlyOptimal;
		PositionRoughnessDrain.LoadOp = RenderInterface::AttachmentLoadOp::Load;
		PositionRoughnessDrain.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;

		Drain NormalMetallicDrain = {};
		NormalMetallicDrain.Name = L"NormalMetallic";
		NormalMetallicDrain.Binding = BindingMode::InputAttachment;
		NormalMetallicDrain.Layout = RenderInterface::ImageLayout::ShaderReadOnlyOptimal;
		NormalMetallicDrain.LoadOp = RenderInterface::AttachmentLoadOp::Load;
		NormalMetallicDrain.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;

		Drain ColorBufferDrain = {};
		ColorBufferDrain.Name = L"ColorBuffer";
		ColorBufferDrain.Binding = BindingMode::ColorAttachment;
		ColorBufferDrain.ClearColor.R =
			ColorBufferDrain.ClearColor.G =
			ColorBufferDrain.ClearColor.B =
			ColorBufferDrain.ClearColor.A = 1.0f;
		ColorBufferDrain.Layout = RenderInterface::ImageLayout::ColorAttachmentOptimal;
		ColorBufferDrain.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		ColorBufferDrain.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;

		Description.Drains = { AlbedoDrain, PositionRoughnessDrain, NormalMetallicDrain, ColorBufferDrain };

		Source ColorBufferSource = { L"ColorBuffer" };

		Description.Sources = { ColorBufferSource };

		Description.Callback.Bind<PBRPass, &PBRPass::PassCallback>(this);
	}

	const PassDesc& PBRPass::GetPassDescription() const
	{
		return Description;
	}

	void PBRPass::PassCallback(RenderInterface::CommandBuffer& CommandBuffer,
	                           const RenderInterface::RenderPass& PassInstance,
	                           const std::vector<std::pair<
		                           const RenderInterface::Image*, const RenderInterface::ImageView*>>& Drains,
	                           const Scene& Scene, bool ResourcesWereRecreated)
	{
		if (ResourcesWereRecreated || !IsPipelineCreated)
		{
			RecreatePipeline(PassInstance);
			IsPipelineCreated = true;
		}

		if (!PrecomputedBRDF || !PrecomputedBRDFSampler)
		{
			EnsurePrecomputedBRDF();
			DescriptorSet->UpdateWithImageAndSampler(6, 0, *PrecomputedBRDFView, *PrecomputedBRDFSampler,
			                                         RenderInterface::ImageLayout::ShaderReadOnlyOptimal);
		}

		// NOTE : 3 input attachments + color buffer
		HERMES_ASSERT_LOG(Drains.size() == 4, L"Invalid attachment count");
		for (uint32 AttachmentIndex = 0; AttachmentIndex < 3; AttachmentIndex++)
		{
			DescriptorSet->UpdateWithImage(AttachmentIndex, 0, *Drains[AttachmentIndex].second,
			                               RenderInterface::ImageLayout::ShaderReadOnlyOptimal);
		}

		DescriptorSet->UpdateWithImageAndSampler(4, 0, Scene.GetIrradianceEnvmap().GetDefaultView(),
		                                         *EnvmapSampler,
		                                         RenderInterface::ImageLayout::ShaderReadOnlyOptimal);
		DescriptorSet->UpdateWithImageAndSampler(5, 0, Scene.GetSpecularEnvmap().GetDefaultView(),
		                                         *EnvmapSampler,
		                                         RenderInterface::ImageLayout::ShaderReadOnlyOptimal);

		LightingData Lighting = {};
		Lighting.PointLightCount = static_cast<uint32>(Scene.GetPointLights().size());
		for (uint32 LightIndex = 0; LightIndex < Lighting.PointLightCount; LightIndex++)
			Lighting.PointLights[LightIndex] = Scene.GetPointLights()[LightIndex];
		Lighting.CameraPosition = Vec4(Scene.GetActiveCamera().GetLocation(), 1.0f);
		Lighting.AmbientLightingCoefficient = DefaultAmbientLightingCoefficient;
		auto* LightingDataMemory = LightingDataUniformBuffer->Map();
		memcpy(LightingDataMemory, &Lighting, sizeof(Lighting));
		LightingDataUniformBuffer->Unmap();

		CommandBuffer.BindPipeline(*Pipeline);
		CommandBuffer.BindDescriptorSet(*DescriptorSet, *Pipeline, 0);
		CommandBuffer.Draw(6, 1, 0, 0);
	}

	void PBRPass::RecreatePipeline(const RenderInterface::RenderPass& Pass)
	{
		RenderInterface::PipelineDescription PipelineDescription = {};

		PipelineDescription.ShaderStages = { VertexShader.get(), FragmentShader.get() };
		PipelineDescription.DescriptorLayouts = { DescriptorLayout.get() };
		PipelineDescription.InputAssembler.Topology = RenderInterface::TopologyType::TriangleList;
		PipelineDescription.Viewport.Origin = { 0, 0 };
		PipelineDescription.Viewport.Dimensions = Renderer::Get().GetSwapchain().GetSize();
		PipelineDescription.Rasterizer.Fill = RenderInterface::FillMode::Fill;
		PipelineDescription.Rasterizer.Cull = RenderInterface::CullMode::Back;
		PipelineDescription.Rasterizer.Direction = RenderInterface::FaceDirection::CounterClockwise;
		PipelineDescription.DepthStencilStage.IsDepthTestEnabled = true;
		PipelineDescription.DepthStencilStage.IsDepthWriteEnabled = false;

		Pipeline = Renderer::Get().GetActiveDevice().CreatePipeline(Pass, PipelineDescription);
	}

	void PBRPass::EnsurePrecomputedBRDF()
	{
		static constexpr Vec2ui Dimensions { 512 };
		static constexpr RenderInterface::DataFormat Format = RenderInterface::DataFormat::R16G16SignedFloat;

		// First, destroy the previous sampler and BRDF image
		PrecomputedBRDF.reset();
		PrecomputedBRDFSampler.reset();

		// Compute the BRDF
		auto& Device = Renderer::Get().GetActiveDevice();

		PrecomputedBRDF = Device.CreateImage(Dimensions,
		                                     RenderInterface::ImageUsageType::ColorAttachment |
		                                     RenderInterface::ImageUsageType::Sampled, Format, 1,
		                                     RenderInterface::ImageLayout::Undefined);
		PrecomputedBRDFView = PrecomputedBRDF->CreateDefaultImageView();

		RenderInterface::RenderPassAttachment OutputAttachment = {};
		OutputAttachment.Type = RenderInterface::AttachmentType::Color;
		OutputAttachment.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		OutputAttachment.StoreOp = RenderInterface::AttachmentStoreOp::Store;
		OutputAttachment.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;
		OutputAttachment.StencilStoreOp = RenderInterface::AttachmentStoreOp::Undefined;
		OutputAttachment.Format = PrecomputedBRDF->GetDataFormat();
		OutputAttachment.LayoutAtStart = RenderInterface::ImageLayout::ColorAttachmentOptimal;
		OutputAttachment.LayoutAtEnd = RenderInterface::ImageLayout::ShaderReadOnlyOptimal;
		auto RenderPass = Device.CreateRenderPass({ OutputAttachment });

		auto VertexShader = Device.CreateShader(L"Shaders/Bin/fs_vert.glsl.spv", RenderInterface::ShaderType::VertexShader);
		auto FragmentShader = Device.CreateShader(L"Shaders/Bin/precompute_brdf.glsl.spv", RenderInterface::ShaderType::FragmentShader);

		RenderInterface::PipelineDescription PipelineDesc = {};
		PipelineDesc.ShaderStages = { VertexShader.get(), FragmentShader.get() };
		PipelineDesc.InputAssembler.Topology = RenderInterface::TopologyType::TriangleList;
		PipelineDesc.Viewport.Origin = { 0 };
		PipelineDesc.Viewport.Dimensions = Dimensions;
		PipelineDesc.Rasterizer.Fill = RenderInterface::FillMode::Fill;
		PipelineDesc.Rasterizer.Cull = RenderInterface::CullMode::Back;
		PipelineDesc.Rasterizer.Direction = RenderInterface::FaceDirection::CounterClockwise;
		PipelineDesc.DepthStencilStage.IsDepthTestEnabled = false;
		PipelineDesc.DepthStencilStage.IsDepthWriteEnabled = false;
		auto Pipeline = Device.CreatePipeline(*RenderPass, PipelineDesc);
		
		auto RenderTarget = Device.CreateRenderTarget(*RenderPass, { PrecomputedBRDFView.get() },
		                                              PrecomputedBRDF->GetSize());

		auto& Queue = Device.GetQueue(RenderInterface::QueueType::Render);
		auto CommandBuffer = Queue.CreateCommandBuffer(true);
		auto Fence = Device.CreateFence();

		/*
		 * NOTE : steps to take:
		 *   1) Transition the precomputed BRDF image to color attachment optimal
		 *	 2) Begin render pass
		 *	 3) Bind pipeline
		 *	 4) Issue draw call for 2 triangles (6 vertices)
		 *	 5) End render pass/recording
		 */
		CommandBuffer->BeginRecording();
		RenderInterface::ImageMemoryBarrier Barrier = {};
		Barrier.OldLayout = RenderInterface::ImageLayout::Undefined;
		Barrier.NewLayout = RenderInterface::ImageLayout::ColorAttachmentOptimal;
		Barrier.OperationsThatHaveToEndBefore = RenderInterface::AccessType::None;
		Barrier.OperationsThatCanStartAfter = RenderInterface::AccessType::ColorAttachmentWrite;
		Barrier.BaseMipLevel = 0;
		Barrier.MipLevelCount = 1;
		CommandBuffer->InsertImageMemoryBarrier(*PrecomputedBRDF, Barrier, RenderInterface::PipelineStage::BottomOfPipe,
		                                        RenderInterface::PipelineStage::ColorAttachmentOutput);
		CommandBuffer->BeginRenderPass(*RenderPass, *RenderTarget, { { 0.0f, 0.0f, 0.0f, 1.0f } });
		CommandBuffer->BindPipeline(*Pipeline);
		CommandBuffer->Draw(6, 1, 0, 0);
		CommandBuffer->EndRenderPass();
		CommandBuffer->EndRecording();

		Queue.SubmitCommandBuffer(*CommandBuffer, Fence.get());

		RenderInterface::SamplerDescription SamplerDesc = {};
		SamplerDesc.AddressingModeU = SamplerDesc.AddressingModeV = RenderInterface::AddressingMode::Repeat;
		SamplerDesc.MinificationFilteringMode = RenderInterface::FilteringMode::Linear;
		SamplerDesc.MagnificationFilteringMode = RenderInterface::FilteringMode::Linear;
		SamplerDesc.CoordinateSystem = RenderInterface::CoordinateSystem::Normalized;
		SamplerDesc.MipMode = RenderInterface::MipmappingMode::Linear;
		SamplerDesc.MinMipLevel = 0.0f;
		SamplerDesc.MaxMipLevel = 1.0;
		SamplerDesc.MipBias = 0.0f;
		PrecomputedBRDFSampler = Device.CreateSampler(SamplerDesc);

		Fence->Wait(UINT64_MAX);
	}
}
