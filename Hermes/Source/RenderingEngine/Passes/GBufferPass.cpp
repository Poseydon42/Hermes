#include "GBufferPass.h"

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

		RenderInterface::DescriptorBinding PerFrameLightingDataUBOBinding = {};
		PerFrameLightingDataUBOBinding.DescriptorCount = 1;
		PerFrameLightingDataUBOBinding.Index = 1;
		PerFrameLightingDataUBOBinding.Type = RenderInterface::DescriptorType::UniformBuffer;
		PerFrameLightingDataUBOBinding.Shader = RenderInterface::ShaderType::FragmentShader;
		PerFrameUBODescriptorLayout = Device->CreateDescriptorSetLayout({ PerFrameVertexShaderUBOBinging, PerFrameLightingDataUBOBinding });
		
		VertexShader = Device->CreateShader(L"Shaders/Bin/basic_vert.glsl.spv", RenderInterface::ShaderType::VertexShader);
		FragmentShader = Device->CreateShader(L"Shaders/Bin/basic_frag.glsl.spv", RenderInterface::ShaderType::FragmentShader);

		SceneDataUniformBuffer = Device->CreateBuffer(
			sizeof(PerFrameSceneUBO),
			RenderInterface::BufferUsageType::CPUAccessible | RenderInterface::BufferUsageType::UniformBuffer);
		LightingDataUniformBuffer = Device->CreateBuffer(
			sizeof(PerFrameLightingUBO),
			RenderInterface::BufferUsageType::CPUAccessible | RenderInterface::BufferUsageType::UniformBuffer);
		PerFrameDataDescriptor = DescriptorAllocator.Allocate(PerFrameUBODescriptorLayout);
		HERMES_ASSERT_LOG(PerFrameDataDescriptor, L"Failed to allocate per frame data descriptor set");
		PerFrameDataDescriptor->UpdateWithBuffer(0, 0, *SceneDataUniformBuffer, 0, sizeof(PerFrameSceneUBO));
		PerFrameDataDescriptor->UpdateWithBuffer(1, 0, *LightingDataUniformBuffer, 0, sizeof(PerFrameLightingUBO));

		PassDesc::RenderPassCallbackType PassCallback;
		PassCallback.Bind<GBufferPass, &GBufferPass::PassCallback>(this);
		Descriptor.Callback = PassCallback;

		Drain GBufferDrain = {};
		GBufferDrain.Name = L"GBuffer";
		GBufferDrain.Binding = BindingMode::ColorAttachment;
		GBufferDrain.ClearColor[0] =
			GBufferDrain.ClearColor[1] =
			GBufferDrain.ClearColor[2] =
			GBufferDrain.ClearColor[3] = 0.0f;
		GBufferDrain.Layout = RenderInterface::ImageLayout::ColorAttachmentOptimal;
		GBufferDrain.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		GBufferDrain.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;

		Drain DepthBufferDrain = {};
		DepthBufferDrain.Name = L"DepthBuffer";
		DepthBufferDrain.Binding = BindingMode::DepthStencilAttachment;
		DepthBufferDrain.ClearColor[0] =
			DepthBufferDrain.ClearColor[1] =
			DepthBufferDrain.ClearColor[2] =
			DepthBufferDrain.ClearColor[3] = 1.0f;
		DepthBufferDrain.Layout = RenderInterface::ImageLayout::DepthStencilAttachmentOptimal;
		DepthBufferDrain.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		DepthBufferDrain.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;

		Descriptor.Drains = { GBufferDrain, DepthBufferDrain };

		Source GBufferSource = {};
		GBufferSource.Name = L"GBuffer";

		Source DepthBufferSource = {};
		DepthBufferSource.Name = L"DepthBuffer";

		Descriptor.Sources = { GBufferSource, DepthBufferSource };
	}

	const PassDesc& GBufferPass::GetPassDescription() const
	{
		return Descriptor;
	}

	void GBufferPass::PassCallback(
		RenderInterface::CommandBuffer& CommandBuffer,
		const RenderInterface::RenderPass& PassInstance,
		const std::vector<const RenderInterface::Image*>&,
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

		PerFrameLightingUBO LightingUBOData;
		LightingUBOData.CameraPosition = Vec4(CurrentCamera.GetLocation(), 1.0f);
		LightingUBOData.PointLightCount = static_cast<uint32>(Scene.GetPointLights().size());
		LightingUBOData.AmbientLightingCoefficient = DefaultAmbientLightingCoefficient;
		if (Scene.GetPointLights().size() > MaxPointLightCount)
			HERMES_LOG_WARNING(L"Scene has %zu point lights, but at most %u are supported.", Scene.GetPointLights().size(), MaxPointLightCount);
		for (uint32 LightIndex = 0;
			LightIndex < Math::Min(MaxPointLightCount, static_cast<uint32>(Scene.GetPointLights().size()));
			LightIndex++)
		{
			LightingUBOData.PointLights[LightIndex] = Scene.GetPointLights()[LightIndex];
		}

		auto LightingUBOMemory = LightingDataUniformBuffer->Map();
		memcpy(LightingUBOMemory, &LightingUBOData, sizeof(LightingUBOData));
		LightingDataUniformBuffer->Unmap();

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

		PipelineDesc.ShaderStages = { VertexShader, FragmentShader };

		PipelineDesc.DescriptorLayouts = { PerFrameUBODescriptorLayout, Material::GetDescriptorSetLayout() };

		RenderInterface::VertexBinding VertexInput = {};
		VertexInput.Index = 0;
		VertexInput.Stride = sizeof(Vertex);
		VertexInput.IsPerInstance = false;
		PipelineDesc.VertexInput.VertexBindings.push_back(VertexInput);

		RenderInterface::VertexAttribute PositionAttribute = {}, TextureCoordinatesAttribute = {}, NormalAttribute = {};
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

		PipelineDesc.InputAssembler.Topology = RenderInterface::TopologyType::TriangleList;

		PipelineDesc.Viewport.Origin = { 0, 0 };
		PipelineDesc.Viewport.Dimensions = Renderer::Get().GetSwapchain().GetSize();

		PipelineDesc.Rasterizer.Cull = RenderInterface::CullMode::Back;
		PipelineDesc.Rasterizer.Direction = RenderInterface::FaceDirection::Clockwise;
		PipelineDesc.Rasterizer.Fill = RenderInterface::FillMode::Fill;

		PipelineDesc.DepthStencilStage.IsDepthTestEnabled = true;
		PipelineDesc.DepthStencilStage.IsDepthWriteEnabled = true;
		PipelineDesc.DepthStencilStage.ComparisonMode = RenderInterface::ComparisonOperator::Less;

		Pipeline = Device->CreatePipeline(Pass, PipelineDesc);
	}
}
