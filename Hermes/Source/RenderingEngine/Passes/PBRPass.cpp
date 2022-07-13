#include "PBRPass.h"

#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Scene/Camera.h"
#include "RenderingEngine/Scene/Scene.h"
#include "RenderInterface/GenericRenderInterface/Buffer.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/Pipeline.h"
#include "RenderInterface/GenericRenderInterface/Shader.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"

namespace Hermes
{
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
		DescriptorLayout = Device.CreateDescriptorSetLayout({
			                                                    AlbedoAttachmentBinding, PositionRoughnessBinding,
			                                                    NormalMetallicBinding, UBOBinding, IrradianceMapBinding
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
		IrradianceMapSampler = Device.CreateSampler(SamplerDescription);

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
		ColorBufferDrain.ClearColor[0] = ColorBufferDrain.ClearColor[1] = ColorBufferDrain.ClearColor[2] =
			ColorBufferDrain.ClearColor[3] = 1.0f;
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

		// NOTE : 3 input attachments + color buffer
		HERMES_ASSERT_LOG(Drains.size() == 4, L"Invalid attachment count");
		for (uint32 AttachmentIndex = 0; AttachmentIndex < 3; AttachmentIndex++)
		{
			DescriptorSet->UpdateWithImage(AttachmentIndex, 0, *Drains[AttachmentIndex].second,
			                               RenderInterface::ImageLayout::ShaderReadOnlyOptimal);
		}

		DescriptorSet->UpdateWithImageAndSampler(4, 0, Scene.GetIrradianceEnvmap().GetDefaultView(),
		                                         *IrradianceMapSampler,
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
}
