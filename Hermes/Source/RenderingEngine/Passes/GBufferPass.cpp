#include "GBufferPass.h"

#include "Logging/Logger.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Scene/Camera.h"
#include "RenderingEngine/Scene/Scene.h"
#include "RenderInterface/GenericRenderInterface/Buffer.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/Pipeline.h"
#include "RenderInterface/GenericRenderInterface/RenderPass.h"
#include "RenderInterface/GenericRenderInterface/Shader.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"

namespace Hermes
{
	GBufferPass::GBufferPass(std::shared_ptr<RenderInterface::Device> InDevice)
		: Device(std::move(InDevice))
	{
		auto& DescriptorAllocator = Renderer::Get().GetDescriptorAllocator();

		RenderInterface::DescriptorBinding PerFrameVertexShaderUBOBinging = {};
		PerFrameVertexShaderUBOBinging.DescriptorCount = 1;
		PerFrameVertexShaderUBOBinging.Index = 0;
		PerFrameVertexShaderUBOBinging.Type = RenderInterface::DescriptorType::UniformBuffer;
		PerFrameVertexShaderUBOBinging.Shader = RenderInterface::ShaderType::VertexShader;
		PerFrameUBODescriptorLayout = Device->CreateDescriptorSetLayout({ PerFrameVertexShaderUBOBinging });
		
		VertexShader = Device->CreateShader(L"Shaders/Bin/gbuffer_vert.glsl.spv", RenderInterface::ShaderType::VertexShader);
		FragmentShader = Device->CreateShader(L"Shaders/Bin/gbuffer_frag.glsl.spv", RenderInterface::ShaderType::FragmentShader);

		SceneDataUniformBuffer = Device->CreateBuffer(
			sizeof(PerFrameSceneUBO),
			RenderInterface::BufferUsageType::CPUAccessible | RenderInterface::BufferUsageType::UniformBuffer);
		PerFrameDataDescriptor = DescriptorAllocator.Allocate(*PerFrameUBODescriptorLayout);
		HERMES_ASSERT_LOG(PerFrameDataDescriptor, L"Failed to allocate per frame data descriptor set");
		PerFrameDataDescriptor->UpdateWithBuffer(0, 0, *SceneDataUniformBuffer, 0, sizeof(PerFrameSceneUBO));

		PassDesc::RenderPassCallbackType PassCallback;
		PassCallback.Bind<GBufferPass, &GBufferPass::PassCallback>(this);
		Descriptor.Callback = PassCallback;

		Drain AlbedoDrain = {};
		AlbedoDrain.Name = L"Albedo";
		AlbedoDrain.Binding = BindingMode::ColorAttachment;
		AlbedoDrain.ClearColor.R =
			AlbedoDrain.ClearColor.G =
			AlbedoDrain.ClearColor.B =
			AlbedoDrain.ClearColor.A = 0.0f;
		AlbedoDrain.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		AlbedoDrain.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;

		Drain PositionRoughnessDrain = {};
		PositionRoughnessDrain.Name = L"PositionRoughness";
		PositionRoughnessDrain.Binding = BindingMode::ColorAttachment;
		PositionRoughnessDrain.ClearColor.R =
			PositionRoughnessDrain.ClearColor.G =
			PositionRoughnessDrain.ClearColor.B =
			PositionRoughnessDrain.ClearColor.A = 0.0f;
		PositionRoughnessDrain.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		PositionRoughnessDrain.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;

		Drain NormalMetallicDrain = {};
		NormalMetallicDrain.Name = L"NormalMetallic";
		NormalMetallicDrain.Binding = BindingMode::ColorAttachment;
		NormalMetallicDrain.ClearColor.R =
			NormalMetallicDrain.ClearColor.G =
			NormalMetallicDrain.ClearColor.B =
			NormalMetallicDrain.ClearColor.A = 0.0f;
		NormalMetallicDrain.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		NormalMetallicDrain.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;

		Drain DepthDrain = {};
		DepthDrain.Name = L"Depth";
		DepthDrain.Binding = BindingMode::DepthStencilAttachment;
		DepthDrain.ClearColor.Depth = 0.0f;
		DepthDrain.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		DepthDrain.StencilLoadOp = RenderInterface::AttachmentLoadOp::Clear;

		Descriptor.Drains = { AlbedoDrain, PositionRoughnessDrain, NormalMetallicDrain, DepthDrain };
	}

	const PassDesc& GBufferPass::GetPassDescription() const
	{
		return Descriptor;
	}

	void GBufferPass::PassCallback(RenderInterface::CommandBuffer& CommandBuffer,
	                               const RenderInterface::RenderPass& PassInstance,
	                               const std::vector<std::pair<
		                               const RenderInterface::Image*, const RenderInterface::ImageView*>>&,
	                               const Scene& Scene, bool ResourcesWereRecreated)
	{
		if (ResourcesWereRecreated || !IsPipelineCreated)
		{
			RecreatePipeline(PassInstance);
			IsPipelineCreated = true;
		}

		auto& CurrentCamera = Scene.GetActiveCamera();

		PerFrameSceneUBO SceneUBOData;
		SceneUBOData.ViewProjection = CurrentCamera.GetProjectionMatrix() * CurrentCamera.GetViewMatrix();

		auto SceneUBOMemory = SceneDataUniformBuffer->Map();
		memcpy(SceneUBOMemory, &SceneUBOData, sizeof(SceneUBOData));
		SceneDataUniformBuffer->Unmap();

		CommandBuffer.BindPipeline(*Pipeline);
		CommandBuffer.BindDescriptorSet(*PerFrameDataDescriptor, *Pipeline, 0);

		// TODO : mesh sorting by material
		for (const auto& Mesh : Scene.GetMeshes())
		{
			if (!Mesh.MeshData.IsReady())
				continue;

			CommandBuffer.BindVertexBuffer(Mesh.MeshData.GetVertexBuffer());
			CommandBuffer.BindIndexBuffer(Mesh.MeshData.GetIndexBuffer(), RenderInterface::IndexSize::Uint32);
			CommandBuffer.BindDescriptorSet(Mesh.Material->GetMaterialDescriptorSet(), *Pipeline, 1);
			CommandBuffer.UploadPushConstants(
				*Pipeline, RenderInterface::ShaderType::VertexShader,
				&Mesh.TransformationMatrix, sizeof(Mesh.TransformationMatrix), 0);

			auto DrawcallInformation = Mesh.MeshData.GetDrawInformation();
			CommandBuffer.DrawIndexed(DrawcallInformation.IndexCount, 1, DrawcallInformation.IndexOffset, DrawcallInformation.VertexOffset, 0);
		}
	}

	void GBufferPass::RecreatePipeline(const RenderInterface::RenderPass& Pass)
	{
		RenderInterface::PipelineDescription PipelineDesc = {};

		RenderInterface::PushConstantRange ModelMatrixRange = {};
		ModelMatrixRange.Size = sizeof(Mat4);
		ModelMatrixRange.Offset = 0;
		ModelMatrixRange.ShadersThatAccess = RenderInterface::ShaderType::VertexShader;
		PipelineDesc.PushConstants.push_back(ModelMatrixRange);

		PipelineDesc.ShaderStages = { VertexShader.get(), FragmentShader.get() };

		PipelineDesc.DescriptorLayouts = { PerFrameUBODescriptorLayout.get(), Material::GetDescriptorSetLayout().get() };

		RenderInterface::VertexBinding VertexInput = {};
		VertexInput.Index = 0;
		VertexInput.Stride = sizeof(Vertex);
		VertexInput.IsPerInstance = false;
		PipelineDesc.VertexInput.VertexBindings.push_back(VertexInput);

		RenderInterface::VertexAttribute PositionAttribute = {}, TextureCoordinatesAttribute = {}, NormalAttribute = {}, TangentAttribute = {};
		PositionAttribute.BindingIndex = 0;
		PositionAttribute.Location = 0;
		PositionAttribute.Offset = offsetof(Vertex, Position);
		PositionAttribute.Format = RenderInterface::DataFormat::R32G32B32SignedFloat;
		PipelineDesc.VertexInput.VertexAttributes.push_back(PositionAttribute);

		TextureCoordinatesAttribute.BindingIndex = 0;
		TextureCoordinatesAttribute.Location = 1;
		TextureCoordinatesAttribute.Offset = offsetof(Vertex, TextureCoordinates);
		TextureCoordinatesAttribute.Format = RenderInterface::DataFormat::R32G32SignedFloat;
		PipelineDesc.VertexInput.VertexAttributes.push_back(TextureCoordinatesAttribute);

		NormalAttribute.BindingIndex = 0;
		NormalAttribute.Location = 2;
		NormalAttribute.Offset = offsetof(Vertex, Normal);
		NormalAttribute.Format = RenderInterface::DataFormat::R32G32B32SignedFloat;
		PipelineDesc.VertexInput.VertexAttributes.push_back(NormalAttribute);

		TangentAttribute.BindingIndex = 0;
		TangentAttribute.Location = 3;
		TangentAttribute.Offset = offsetof(Vertex, Tangent);
		TangentAttribute.Format = RenderInterface::DataFormat::R32G32B32SignedFloat;
		PipelineDesc.VertexInput.VertexAttributes.push_back(TangentAttribute);

		PipelineDesc.InputAssembler.Topology = RenderInterface::TopologyType::TriangleList;

		PipelineDesc.Viewport.Origin = { 0, 0 };
		PipelineDesc.Viewport.Dimensions = Renderer::Get().GetSwapchain().GetSize();

		PipelineDesc.Rasterizer.Cull = RenderInterface::CullMode::Back;
		PipelineDesc.Rasterizer.Direction = RenderInterface::FaceDirection::Clockwise;
		PipelineDesc.Rasterizer.Fill = RenderInterface::FillMode::Fill;

		PipelineDesc.DepthStencilStage.IsDepthTestEnabled = true;
		PipelineDesc.DepthStencilStage.IsDepthWriteEnabled = true;
		PipelineDesc.DepthStencilStage.ComparisonMode = RenderInterface::ComparisonOperator::Greater;

		Pipeline = Device->CreatePipeline(Pass, PipelineDesc);
	}
}
