#include "Renderer.h"

#include "AssetSystem/MeshAsset.h"
#include "RenderingEngine/Scene/Scene.h"
#include "RenderInterface/GenericRenderInterface/CommandBuffer.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"
#include "RenderInterface/GenericRenderInterface/Pipeline.h"
#include "RenderInterface/GenericRenderInterface/RenderPass.h"
#include "RenderInterface/GenericRenderInterface/CommonTypes.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"

namespace Hermes
{
	Renderer& Renderer::Get()
	{
		static Renderer Instance;
		return Instance;
	}

	bool Renderer::Init(const RenderInterface::PhysicalDevice& GPU)
	{
		RenderingDevice = GPU.CreateDevice();
		if (!RenderingDevice)
			return false;
		GPUProperties = GPU.GetProperties();
		DumpGPUProperties();

		Swapchain = RenderingDevice->CreateSwapchain(NumberOfBackBuffers);
		if (!Swapchain)
			return false;

		RenderInterface::DescriptorBinding PerFrameVertexShaderUBOBinging;
		PerFrameVertexShaderUBOBinging.DescriptorCount = 1;
		PerFrameVertexShaderUBOBinging.Index = 0;
		PerFrameVertexShaderUBOBinging.Type = RenderInterface::DescriptorType::UniformBuffer;
		PerFrameVertexShaderUBOBinging.Shader = RenderInterface::ShaderType::VertexShader;

		RenderInterface::DescriptorBinding PerFrameLightingDataUBOBinding;
		PerFrameLightingDataUBOBinding.DescriptorCount = 1;
		PerFrameLightingDataUBOBinding.Index = 1;
		PerFrameLightingDataUBOBinding.Type = RenderInterface::DescriptorType::UniformBuffer;
		PerFrameLightingDataUBOBinding.Shader = RenderInterface::ShaderType::FragmentShader;
		PerFrameUBODescriptorLayout = RenderingDevice->CreateDescriptorSetLayout({ PerFrameVertexShaderUBOBinging, PerFrameLightingDataUBOBinding });

		RenderInterface::SubpoolDescription PerFrameUBOSubpoolDesc;
		PerFrameUBOSubpoolDesc.Count = NumberOfBackBuffers * 2;
		PerFrameUBOSubpoolDesc.Type = RenderInterface::DescriptorType::UniformBuffer;
		PerFrameUBODescriptorPool = RenderingDevice->CreateDescriptorSetPool(NumberOfBackBuffers, { PerFrameUBOSubpoolDesc });

		VertexShader = RenderingDevice->CreateShader(L"Shaders/Bin/basic_vert.glsl.spv", RenderInterface::ShaderType::VertexShader);
		FragmentShader = RenderingDevice->CreateShader(L"Shaders/Bin/basic_frag.glsl.spv", RenderInterface::ShaderType::FragmentShader);

		SceneDataUniformBuffer = RenderingDevice->CreateBuffer(
				sizeof(PerFrameSceneUBO),
				RenderInterface::BufferUsageType::CPUAccessible | RenderInterface::BufferUsageType::UniformBuffer);
		LightingDataUniformBuffer = RenderingDevice->CreateBuffer(
				sizeof(PerFrameLightingUBO),
				RenderInterface::BufferUsageType::CPUAccessible | RenderInterface::BufferUsageType::UniformBuffer);
		PerFrameDataDescriptor = PerFrameUBODescriptorPool->CreateDescriptorSet(PerFrameUBODescriptorLayout);
		PerFrameDataDescriptor->UpdateWithBuffer(0, 0, *SceneDataUniformBuffer, 0, sizeof(PerFrameSceneUBO));
		PerFrameDataDescriptor->UpdateWithBuffer(1, 0, *LightingDataUniformBuffer, 0, sizeof(PerFrameLightingUBO));

		FrameGraphScheme Scheme;
		PassDesc GraphicsPass;
		PassDesc::RenderPassCallbackType PassCallback;
		PassCallback.Bind<Renderer, &Renderer::GraphicsPassCallback>(this);
		GraphicsPass.Callback = PassCallback;

		Drain GBufferDrain = {};
		GBufferDrain.Name = L"GBuffer";
		GBufferDrain.Binding = BindingMode::ColorAttachment;
		GBufferDrain.ClearColor[0] =
			GBufferDrain.ClearColor[1] =
			GBufferDrain.ClearColor[2] =
			GBufferDrain.ClearColor[3] = 0.0f;
		GBufferDrain.Format = RenderInterface::DataFormat::B8G8R8A8UnsignedNormalized;
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
		DepthBufferDrain.Format = RenderInterface::DataFormat::D24UnsignedNormalizedS8UnsignedInteger;
		DepthBufferDrain.Layout = RenderInterface::ImageLayout::DepthStencilAttachmentOptimal;
		DepthBufferDrain.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		DepthBufferDrain.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;

		GraphicsPass.Drains = { GBufferDrain, DepthBufferDrain };

		Source GBufferSource = {};
		GBufferSource.Name = L"GBuffer";
		GBufferSource.Format = RenderInterface::DataFormat::B8G8R8A8UnsignedNormalized;

		GraphicsPass.Sources = { GBufferSource };

		Scheme.AddPass(L"GraphicsPass", GraphicsPass);

		ResourceDesc BackbufferResource = {};
		BackbufferResource.Dimensions = SwapchainRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		BackbufferResource.Format = RenderInterface::DataFormat::B8G8R8A8UnsignedNormalized;
		BackbufferResource.MipLevels = 1;
		Scheme.AddResource(L"Backbuffer", BackbufferResource);

		ResourceDesc DepthBufferResource = {};
		DepthBufferResource.Format = RenderInterface::DataFormat::D24UnsignedNormalizedS8UnsignedInteger;
		DepthBufferResource.Dimensions = SwapchainRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		DepthBufferResource.MipLevels = 1;
		Scheme.AddResource(L"DepthBuffer", DepthBufferResource);

		Scheme.AddLink(L"$.Backbuffer", L"GraphicsPass.GBuffer");
		Scheme.AddLink(L"$.DepthBuffer", L"GraphicsPass.DepthBuffer");
		Scheme.AddLink(L"GraphicsPass.GBuffer", L"$.BLIT_TO_SWAPCHAIN");

		FrameGraph = Scheme.Compile();


		RenderInterface::PipelineDescription PipelineDesc = {};

		RenderInterface::PushConstantRange ModelMatrixRange;
		ModelMatrixRange.Size = sizeof(Mat4);
		ModelMatrixRange.Offset = 0;
		ModelMatrixRange.ShadersThatAccess = RenderInterface::ShaderType::VertexShader;
		PipelineDesc.PushConstants.push_back(ModelMatrixRange);

		PipelineDesc.ShaderStages = { VertexShader, FragmentShader };

		PipelineDesc.DescriptorLayouts = { PerFrameUBODescriptorLayout, Material::GetDescriptorSetLayout() };

		RenderInterface::VertexBinding VertexInput;
		VertexInput.Index = 0;
		VertexInput.Stride = sizeof(Vertex);
		VertexInput.IsPerInstance = false;
		PipelineDesc.VertexInput.VertexBindings.push_back(VertexInput);

		RenderInterface::VertexAttribute PositionAttribute, TextureCoordinatesAttribute, NormalAttribute;
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
		PipelineDesc.Viewport.Dimensions = Swapchain->GetSize();

		PipelineDesc.Rasterizer.Cull = RenderInterface::CullMode::Back;
		PipelineDesc.Rasterizer.Direction = RenderInterface::FaceDirection::Clockwise;
		PipelineDesc.Rasterizer.Fill = RenderInterface::FillMode::Fill;

		PipelineDesc.DepthStencilStage.IsDepthTestEnabled = true;
		PipelineDesc.DepthStencilStage.IsDepthWriteEnabled = true;
		PipelineDesc.DepthStencilStage.ComparisonMode = RenderInterface::ComparisonOperator::Less;

		Pipeline = RenderingDevice->CreatePipeline(FrameGraph->GetRenderPassObject(L"GraphicsPass"), PipelineDesc);

		return true;
	}

	void Renderer::UpdateGraphicsSettings(GraphicsSettings NewSettings)
	{
		if (NewSettings.AnisotropyLevel != CurrentSettings.AnisotropyLevel)
		{
			Material::SetDefaultAnisotropyLevel(NewSettings.AnisotropyLevel);
		}

		CurrentSettings = NewSettings;
	}

	void Renderer::RunFrame(const Scene& Scene)
	{
		FrameGraph->Execute(Scene);
	}

	RenderInterface::Device& Renderer::GetActiveDevice()
	{
		return *RenderingDevice;
	}

	RenderInterface::Swapchain& Renderer::GetSwapchain()
	{
		return *Swapchain;
	}

	void Renderer::GraphicsPassCallback(RenderInterface::CommandBuffer& CommandBuffer, const Scene& Scene)
	{
		PerFrameSceneUBO SceneUBOData;
		auto PerspectiveMatrix = Mat4::Perspective(
			VerticalFOV, static_cast<float>(Swapchain->GetSize().X) / static_cast<float>(Swapchain->GetSize().Y),
			NearPlane, FarPlane);
		SceneUBOData.ViewProjection = PerspectiveMatrix * Scene.GetViewMatrix();

		auto SceneUBOMemory = SceneDataUniformBuffer->Map();
		memcpy(SceneUBOMemory, &SceneUBOData, sizeof(SceneUBOData));
		SceneDataUniformBuffer->Unmap();

		PerFrameLightingUBO LightingUBOData;
		LightingUBOData.CameraPosition = Scene.GetCameraPosition();
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

		CommandBuffer.BindPipeline(Pipeline);
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

	void Renderer::DumpGPUProperties() const
	{
		HERMES_LOG_INFO(L"======== GPU Information ========");
		HERMES_LOG_INFO(L"Name: %S", GPUProperties.Name.c_str());
		HERMES_LOG_INFO(L"Anisotropy: %s, %f", GPUProperties.AnisotropySupport ? L"true" : L"false", GPUProperties.MaxAnisotropyLevel);
	}
}
