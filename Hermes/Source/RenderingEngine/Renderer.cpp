#include "Renderer.h"

#include "AssetSystem/MeshAsset.h"
#include "RenderingEngine/Scene/Scene.h"
#include "RenderInterface/GenericRenderInterface/CommandBuffer.h"
#include "RenderInterface/GenericRenderInterface/Fence.h"
#include "RenderInterface/GenericRenderInterface/Image.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"
#include "RenderInterface/GenericRenderInterface/Pipeline.h"
#include "RenderInterface/GenericRenderInterface/RenderPass.h"
#include "RenderingEngine/Scene/Scene.h"

namespace Hermes
{
	Renderer::Renderer()
		: SwapchainWasRecreated(false)
	{
	}

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

		SwapchainImageAcquiredFence = RenderingDevice->CreateFence();
		RenderingFinishedFence = RenderingDevice->CreateFence();

		RecreatePerFrameObjects();

		return true;
	}

	void Renderer::RunFrame(const Scene& Scene)
	{
		auto BackBufferIndex = Swapchain->AcquireImage(UINT64_MAX, *SwapchainImageAcquiredFence, SwapchainWasRecreated);
		if (SwapchainWasRecreated)
			RecreatePerFrameObjects();
		if (!BackBufferIndex.has_value())
		{
			HERMES_LOG_WARNING(L"Swapchain::AcquireImage did not return valid image index. Current frame will be skipped");
			return;
		}

		auto CurrentSceneUniformBuffer = PerFrameObjects[BackBufferIndex.value()].SceneDataUniformBuffer;
		PerFrameSceneUBO SceneUBOData;
		auto PerspectiveMatrix = Mat4::Perspective(
			VerticalFOV, static_cast<float>(Swapchain->GetSize().X) / static_cast<float>(Swapchain->GetSize().Y),
			NearPlane, FarPlane);
		SceneUBOData.ViewProjection = PerspectiveMatrix * Scene.GetViewMatrix();

		auto SceneUBOMemory = CurrentSceneUniformBuffer->Map();
		memcpy(SceneUBOMemory, &SceneUBOData, sizeof(SceneUBOData));
		CurrentSceneUniformBuffer->Unmap();

		auto CurrentLightingUniformBuffer = PerFrameObjects[BackBufferIndex.value()].LightingDataUniformBuffer;
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

		auto LightingUBOMemory = CurrentLightingUniformBuffer->Map();
		memcpy(LightingUBOMemory, &LightingUBOData, sizeof(LightingUBOData));
		CurrentLightingUniformBuffer->Unmap();

		auto CurrentCommandBuffer = PerFrameObjects[BackBufferIndex.value()].CommandBuffer;
		CurrentCommandBuffer->BeginRecording();

		RenderInterface::ImageMemoryBarrier BeforeRenderingImageTransition = {};
		BeforeRenderingImageTransition.OldLayout = RenderInterface::ImageLayout::Undefined;
		BeforeRenderingImageTransition.NewLayout = RenderInterface::ImageLayout::ColorAttachmentOptimal;
		BeforeRenderingImageTransition.OperationsThatHaveToEndBefore = RenderInterface::AccessType::TransferRead;
		BeforeRenderingImageTransition.OperationsThatCanStartAfter = RenderInterface::AccessType::ColorAttachmentWrite;

		CurrentCommandBuffer->InsertImageMemoryBarrier(
			*PerFrameObjects[BackBufferIndex.value()].ColorBuffer, BeforeRenderingImageTransition,
			RenderInterface::PipelineStage::Transfer, RenderInterface::PipelineStage::ColorAttachmentOutput);

		auto CurrentRenderTarget = PerFrameObjects[BackBufferIndex.value()].RenderTarget;
		RenderInterface::ClearColor ColorAttachmentClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		RenderInterface::ClearColor DepthAttachmentClearColor = { 1.0f, 0.0f, 0.0f, 0.0f };
		CurrentCommandBuffer->BeginRenderPass(RenderPass, CurrentRenderTarget, { ColorAttachmentClearColor, DepthAttachmentClearColor });

		CurrentCommandBuffer->BindPipeline(Pipeline);
		CurrentCommandBuffer->BindDescriptorSet(*PerFrameObjects[BackBufferIndex.value()].PerFrameDataDescriptor, *Pipeline, 0);

		// TODO : mesh sorting by material
		for (const auto& Mesh : Scene.GetMeshes())
		{
			if (!Mesh.MeshData.IsReady())
				continue;

			CurrentCommandBuffer->BindVertexBuffer(Mesh.MeshData.GetVertexBuffer());
			CurrentCommandBuffer->BindIndexBuffer(Mesh.MeshData.GetIndexBuffer(), RenderInterface::IndexSize::Uint32);
			CurrentCommandBuffer->BindDescriptorSet(Mesh.Material->GetMaterialDescriptorSet(), *Pipeline, 1);
			CurrentCommandBuffer->UploadPushConstants(
				*Pipeline, RenderInterface::ShaderType::VertexShader,
				&Mesh.TransformationMatrix, sizeof(Mesh.TransformationMatrix), 0);

			auto DrawcallInformation = Mesh.MeshData.GetDrawInformation();
			CurrentCommandBuffer->DrawIndexed(DrawcallInformation.IndexCount, 1, DrawcallInformation.IndexOffset, DrawcallInformation.VertexOffset, 0);
		}

		CurrentCommandBuffer->EndRenderPass();

		RenderInterface::ImageMemoryBarrier SwapchainImageToCopyDestination = {};
		SwapchainImageToCopyDestination.OldLayout = RenderInterface::ImageLayout::Undefined;
		SwapchainImageToCopyDestination.NewLayout = RenderInterface::ImageLayout::TransferDestinationOptimal;
		SwapchainImageToCopyDestination.OperationsThatHaveToEndBefore = RenderInterface::AccessType::MemoryRead;
		SwapchainImageToCopyDestination.OperationsThatCanStartAfter = RenderInterface::AccessType::TransferWrite;
		CurrentCommandBuffer->InsertImageMemoryBarrier(
			*Swapchain->GetImage(BackBufferIndex.value()), SwapchainImageToCopyDestination,
			RenderInterface::PipelineStage::BottomOfPipe, RenderInterface::PipelineStage::Transfer);

		RenderInterface::ImageCopyRegion CopyToSwapchain = {};
		CopyToSwapchain.Dimensions = Swapchain->GetSize();
		CurrentCommandBuffer->CopyImage(
			*PerFrameObjects[BackBufferIndex.value()].ColorBuffer, RenderInterface::ImageLayout::TransferSourceOptimal,
			*Swapchain->GetImage(BackBufferIndex.value()), RenderInterface::ImageLayout::TransferDestinationOptimal,
			{ CopyToSwapchain });

		RenderInterface::ImageMemoryBarrier SwapchainImageToPresentationReady = {};
		SwapchainImageToPresentationReady.OldLayout = RenderInterface::ImageLayout::TransferDestinationOptimal;
		SwapchainImageToPresentationReady.NewLayout = RenderInterface::ImageLayout::ReadyForPresentation;
		SwapchainImageToPresentationReady.OperationsThatHaveToEndBefore = RenderInterface::AccessType::TransferWrite;
		SwapchainImageToPresentationReady.OperationsThatCanStartAfter = RenderInterface::AccessType::MemoryRead;
		CurrentCommandBuffer->InsertImageMemoryBarrier(
			*Swapchain->GetImage(BackBufferIndex.value()), SwapchainImageToPresentationReady,
			RenderInterface::PipelineStage::Transfer, RenderInterface::PipelineStage::TopOfPipe);

		CurrentCommandBuffer->EndRecording();

		auto& RenderQueue = RenderingDevice->GetQueue(RenderInterface::QueueType::Render);
		RenderQueue.SubmitCommandBuffer(CurrentCommandBuffer, RenderingFinishedFence);

		SwapchainImageAcquiredFence->Wait(UINT64_MAX);
		RenderingFinishedFence->Wait(UINT64_MAX);
		SwapchainImageAcquiredFence->Reset();
		RenderingFinishedFence->Reset();

		Swapchain->Present(BackBufferIndex.value(), SwapchainWasRecreated);
	}

	RenderInterface::Device& Renderer::GetActiveDevice()
	{
		return *RenderingDevice;
	}

	void Renderer::RecreatePerFrameObjects()
	{
		RenderInterface::RenderPassDescription RenderPassDesc = {};

		RenderInterface::RenderPassAttachment ColorAttachment = {};
		ColorAttachment.Format = ColorAttachmentFormat;
		ColorAttachment.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		ColorAttachment.StoreOp = RenderInterface::AttachmentStoreOp::Store;
		ColorAttachment.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;
		ColorAttachment.StencilStoreOp = RenderInterface::AttachmentStoreOp::Undefined;
		ColorAttachment.LayoutAtStart = RenderInterface::ImageLayout::ColorAttachmentOptimal;
		ColorAttachment.LayoutAtEnd = RenderInterface::ImageLayout::TransferSourceOptimal;
		RenderPassDesc.ColorAttachments.push_back(ColorAttachment);

		RenderInterface::RenderPassAttachment DepthAttachment = {};
		DepthAttachment.Format = DepthAttachmentFormat;
		DepthAttachment.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		DepthAttachment.StoreOp = RenderInterface::AttachmentStoreOp::Store;
		DepthAttachment.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;
		DepthAttachment.StencilStoreOp = RenderInterface::AttachmentStoreOp::Undefined;
		DepthAttachment.LayoutAtStart = RenderInterface::ImageLayout::DepthStencilAttachmentOptimal;
		DepthAttachment.LayoutAtEnd = RenderInterface::ImageLayout::DepthStencilAttachmentOptimal;
		RenderPassDesc.DepthAttachment = DepthAttachment;

		RenderPass = RenderingDevice->CreateRenderPass(RenderPassDesc);

		PerFrameObjects.clear();
		PerFrameObjects.resize(Swapchain->GetImageCount());
		for (auto& Object : PerFrameObjects)
		{
			Object.CommandBuffer = RenderingDevice->GetQueue(RenderInterface::QueueType::Render).CreateCommandBuffer(true);
			Object.SceneDataUniformBuffer = RenderingDevice->CreateBuffer(
				sizeof(PerFrameSceneUBO),
				RenderInterface::BufferUsageType::CPUAccessible | RenderInterface::BufferUsageType::UniformBuffer);
			Object.LightingDataUniformBuffer = RenderingDevice->CreateBuffer(
				sizeof(PerFrameLightingUBO),
				RenderInterface::BufferUsageType::CPUAccessible | RenderInterface::BufferUsageType::UniformBuffer);
			Object.PerFrameDataDescriptor = PerFrameUBODescriptorPool->CreateDescriptorSet(PerFrameUBODescriptorLayout);

			Vec2ui FramebufferDimensions = Swapchain->GetSize();
			Object.ColorBuffer = RenderingDevice->CreateImage(
				FramebufferDimensions, RenderInterface::ImageUsageType::ColorAttachment | RenderInterface::ImageUsageType::CopySource,
				ColorAttachmentFormat, 1, RenderInterface::ImageLayout::Undefined);
			Object.DepthBuffer = RenderingDevice->CreateImage(
				FramebufferDimensions, RenderInterface::ImageUsageType::DepthStencilAttachment | RenderInterface::ImageUsageType::CopySource,
				DepthAttachmentFormat, 1, RenderInterface::ImageLayout::Undefined);

			Object.RenderTarget = RenderingDevice->CreateRenderTarget(RenderPass, { Object.ColorBuffer, Object.DepthBuffer }, FramebufferDimensions);

			Object.PerFrameDataDescriptor->UpdateWithBuffer(0, 0, *Object.SceneDataUniformBuffer, 0, sizeof(PerFrameSceneUBO));
			Object.PerFrameDataDescriptor->UpdateWithBuffer(1, 0, *Object.LightingDataUniformBuffer, 0, sizeof(PerFrameLightingUBO));
		}

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

		Pipeline = RenderingDevice->CreatePipeline(RenderPass, PipelineDesc);
	}
}
