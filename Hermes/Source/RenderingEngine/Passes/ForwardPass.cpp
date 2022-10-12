#include "ForwardPass.h"

#include <optick.h>

#include "Logging/Logger.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Material/Material.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Scene/Camera.h"
#include "RenderingEngine/Scene/Scene.h"
#include "RenderInterface/GenericRenderInterface/Buffer.h"
#include "RenderInterface/GenericRenderInterface/CommandBuffer.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/Fence.h"
#include "RenderInterface/GenericRenderInterface/Pipeline.h"
#include "RenderInterface/GenericRenderInterface/Queue.h"
#include "RenderInterface/GenericRenderInterface/Shader.h"
#include "RenderingEngine/SharedData.h"

namespace Hermes
{
	std::unique_ptr<RenderInterface::Image> ForwardPass::PrecomputedBRDFImage;
	std::unique_ptr<RenderInterface::ImageView> ForwardPass::PrecomputedBRDFView;
	std::unique_ptr<RenderInterface::Sampler> ForwardPass::PrecomputedBRDFSampler;

	ForwardPass::ForwardPass()
	{
		auto& Device = Renderer::Get().GetActiveDevice();

		SceneUBODescriptorSet = Renderer::Get().GetDescriptorAllocator().
		                                        Allocate(Renderer::Get().GetGlobalDataDescriptorSetLayout());
		SceneUBOBuffer = Device.CreateBuffer(sizeof(GlobalSceneData),
		                                     RenderInterface::BufferUsageType::UniformBuffer |
		                                     RenderInterface::BufferUsageType::CPUAccessible);

		RenderInterface::SamplerDescription SamplerDesc = {};
		SamplerDesc.AddressingModeU = SamplerDesc.AddressingModeV = RenderInterface::AddressingMode::Repeat;
		SamplerDesc.MinificationFilteringMode = SamplerDesc.MagnificationFilteringMode = RenderInterface::FilteringMode::Linear;
		SamplerDesc.CoordinateSystem = RenderInterface::CoordinateSystem::Normalized;
		SamplerDesc.MipMode = RenderInterface::MipmappingMode::Linear;
		SamplerDesc.MinMipLevel = 0.0f;
		SamplerDesc.MaxMipLevel = 16.0f; // NOTE : 65536 * 65536 textures is more than enough :)
		SamplerDesc.MipBias = 0.0f;
		EnvmapSampler = Device.CreateSampler(SamplerDesc);

		VertexShader = Device.CreateShader(L"Shaders/Bin/forward_vert.glsl.spv",
		                                   RenderInterface::ShaderType::VertexShader);
		FragmentShader = Device.CreateShader(L"Shaders/Bin/forward_frag.glsl.spv",
		                                     RenderInterface::ShaderType::FragmentShader);

		Description.Callback.Bind<ForwardPass, &ForwardPass::PassCallback>(this);

		Attachment Color = {};
		Color.Name = L"Color";
		Color.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		Color.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;
		Color.ClearColor.R = Color.ClearColor.G = Color.ClearColor.B = Color.ClearColor.A = 1.0f;
		Color.Binding = BindingMode::ColorAttachment;
		Attachment Depth = {};
		Depth.Name = L"Depth";
		Depth.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		Depth.StencilLoadOp = RenderInterface::AttachmentLoadOp::Clear;
		Depth.ClearColor.Depth = 0.0f;
		Depth.Binding = BindingMode::DepthStencilAttachment;
		Description.Attachments = { Color, Depth };
	}

	const PassDesc& ForwardPass::GetPassDescription() const
	{
		return Description;
	}

	void ForwardPass::PassCallback(RenderInterface::CommandBuffer& CommandBuffer,
	                               const RenderInterface::RenderPass&,
	                               const std::vector<std::pair<
		                               const RenderInterface::Image*, const RenderInterface::ImageView*>>&,
	                               const Scene& Scene, bool)
	{
		OPTICK_EVENT();

		if (!PrecomputedBRDFSampler || !PrecomputedBRDFView || !PrecomputedBRDFSampler)
		{
			EnsurePrecomputedBRDF();
		}

		auto& Camera = Scene.GetActiveCamera();
		auto ViewProjectionMatrix = Camera.GetProjectionMatrix() * Camera.GetViewMatrix();

		GlobalSceneData SceneData = {};
		SceneData.ViewProjection = ViewProjectionMatrix;
		SceneData.CameraLocation = Vec4(Camera.GetLocation(), 1.0f);
		HERMES_ASSERT_LOG(Scene.GetPointLights().size() < GlobalSceneData::MaxPointLightCount,
		                  L"There are more point lights in the scene than the shader can process, some of them will be ignored");
		SceneData.PointLightCount = Math::Min<uint32>(static_cast<uint32>(Scene.GetPointLights().size()),
		                                              GlobalSceneData::MaxPointLightCount);
		for (uint32 LightIndex = 0; LightIndex < SceneData.PointLightCount; LightIndex++)
		{
			SceneData.PointLights[LightIndex].Color = Scene.GetPointLights()[LightIndex].Color;
			SceneData.PointLights[LightIndex].Position = Scene.GetPointLights()[LightIndex].Position;
		}

		auto* SceneDataMemory = SceneUBOBuffer->Map();
		memcpy(SceneDataMemory, &SceneData, sizeof(SceneData));
		SceneUBOBuffer->Unmap();

		SceneUBODescriptorSet->UpdateWithBuffer(0, 0, *SceneUBOBuffer, 0,
		                                        static_cast<uint32>(SceneUBOBuffer->GetSize()));
		SceneUBODescriptorSet->UpdateWithImageAndSampler(1, 0, Scene.GetIrradianceEnvmap().GetDefaultView(), *EnvmapSampler,
		                                                 RenderInterface::ImageLayout::ShaderReadOnlyOptimal);
		SceneUBODescriptorSet->UpdateWithImageAndSampler(2, 0, Scene.GetSpecularEnvmap().GetDefaultView(), *EnvmapSampler,
		                                                 RenderInterface::ImageLayout::ShaderReadOnlyOptimal);
		SceneUBODescriptorSet->UpdateWithImageAndSampler(3, 0, *PrecomputedBRDFView, *PrecomputedBRDFSampler,
		                                                 RenderInterface::ImageLayout::ShaderReadOnlyOptimal);

		for (const auto& Mesh : Scene.GetMeshes())
		{
			auto& Material = Mesh.Material;
			auto& MaterialPipeline = Material->GetBaseMaterial().GetPipeline();
			auto& MeshBuffer = Mesh.MeshData;

			// TODO : move it into the renderer?
			Material->PrepareForRender();

			CommandBuffer.BindPipeline(MaterialPipeline);
			CommandBuffer.BindDescriptorSet(*SceneUBODescriptorSet, MaterialPipeline, 0);
			CommandBuffer.BindDescriptorSet(Material->GetMaterialDescriptorSet(), MaterialPipeline, 1);
			CommandBuffer.BindVertexBuffer(MeshBuffer.GetVertexBuffer());
			CommandBuffer.BindIndexBuffer(MeshBuffer.GetIndexBuffer(), RenderInterface::IndexSize::Uint32);
			CommandBuffer.UploadPushConstants(MaterialPipeline, RenderInterface::ShaderType::VertexShader,
			                                  &Mesh.TransformationMatrix, sizeof(Mesh.TransformationMatrix), 0);
			const auto& DrawInformation = MeshBuffer.GetDrawInformation();
			CommandBuffer.DrawIndexed(DrawInformation.IndexCount, 1, DrawInformation.IndexOffset,
			                          DrawInformation.VertexOffset, 0);
		}
	}

	void ForwardPass::EnsurePrecomputedBRDF()
	{
		static constexpr Vec2ui Dimensions { 512 };
		static constexpr RenderInterface::DataFormat Format = RenderInterface::DataFormat::R16G16SignedFloat;

		// First, destroy the previous sampler and BRDF image
		PrecomputedBRDFImage.reset();
		PrecomputedBRDFView.reset();
		PrecomputedBRDFSampler.reset();

		// Compute the BRDF
		auto& Device = Renderer::Get().GetActiveDevice();

		PrecomputedBRDFImage = Device.CreateImage(Dimensions,
		                                     RenderInterface::ImageUsageType::ColorAttachment |
		                                     RenderInterface::ImageUsageType::Sampled, Format, 1,
		                                     RenderInterface::ImageLayout::Undefined);
		PrecomputedBRDFView = PrecomputedBRDFImage->CreateDefaultImageView();

		RenderInterface::RenderPassAttachment OutputAttachment = {};
		OutputAttachment.Type = RenderInterface::AttachmentType::Color;
		OutputAttachment.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		OutputAttachment.StoreOp = RenderInterface::AttachmentStoreOp::Store;
		OutputAttachment.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;
		OutputAttachment.StencilStoreOp = RenderInterface::AttachmentStoreOp::Undefined;
		OutputAttachment.Format = PrecomputedBRDFImage->GetDataFormat();
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
		                                              PrecomputedBRDFImage->GetSize());

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
		CommandBuffer->InsertImageMemoryBarrier(*PrecomputedBRDFImage, Barrier, RenderInterface::PipelineStage::BottomOfPipe,
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
