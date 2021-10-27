#include "AssetSystem/AssetLoader.h"
#include "AssetSystem/ImageAsset.h"
#include "AssetSystem/MeshAsset.h"
#include "Core/Application/InputEngine.h"
#include "Math/Common.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"
#include "Platform/GenericPlatform/PlatformFile.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
#include "RenderInterface/GenericRenderInterface/Sampler.h"
#ifdef HERMES_PLATFORM_WINDOWS

#include "Core/Core.h"
#include "Core/Application/Application.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
#include "Platform/GenericPlatform/PlatformLibrary.h"
#include "RenderInterface/GenericRenderInterface/Buffer.h"
#include "RenderInterface/GenericRenderInterface/Instance.h"
#include "RenderInterface/Vulkan/VulkanInstance.h"
#include "RenderInterface/Vulkan/VulkanSwapchain.h"
#include "Core/Application/EventQueue.h"
#include "RenderInterface/GenericRenderInterface/CommandBuffer.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/Fence.h"
#include "RenderInterface/GenericRenderInterface/RenderPass.h"
#include "RenderInterface/GenericRenderInterface/Pipeline.h"
#include "Core/Application/GameLoop.h"
#include <sstream>

class SandboxApp : public Hermes::IApplication
{
public:

	bool Init() override
	{
		ApplicationWindow = Hermes::GGameLoop->GetWindow();
		
		auto RenderInstance = Hermes::RenderInterface::Instance::CreateRenderInterfaceInstance(ApplicationWindow);
		auto Devices = RenderInstance->EnumerateAvailableDevices();
		auto PhysicalDevice = RenderInstance->GetPhysicalDevice(Devices[0].InternalIndex);
		Device = PhysicalDevice->CreateDevice();
		Swapchain = Device->CreateSwapchain(3);

		auto SuzanneAsset = Hermes::AssetLoader::Load(L"suzanne");
		auto SuzanneMesh = Hermes::Asset::As<Hermes::MeshAsset>(Hermes::AssetLoader::Load(L"suzanne"));
		DrawIndexCount = SuzanneMesh->GetIndexCount();
		
		auto CheckerImageAsset = Hermes::Asset::As<Hermes::ImageAsset>(Hermes::AssetLoader::Load(L"checker_colored"));
		auto CheckerImage = CheckerImageAsset->GetRawData();
		Texture = Device->CreateImage(
			CheckerImageAsset->GetDimensions(), Hermes::RenderInterface::ImageUsageType::CopyDestination | Hermes::RenderInterface::ImageUsageType::Sampled,
			Hermes::RenderInterface::DataFormat::B8G8R8A8UnsignedNormalized, 1, Hermes::RenderInterface::ImageLayout::Undefined);
		auto ImageSizeInBytes = static_cast<Hermes::uint32>(CheckerImageAsset->GetMemorySize());

		Hermes::uint32 StagingBufferSize = Hermes::Math::Max(
			Hermes::Math::Max(
					static_cast<Hermes::uint32>(SuzanneMesh->GetRequiredVertexBufferSize()),
					static_cast<Hermes::uint32>(SuzanneMesh->GetRequiredIndexBufferSize())),
			ImageSizeInBytes);
		auto StagingBuffer = Device->CreateBuffer(StagingBufferSize, Hermes::RenderInterface::BufferUsageType::CPUAccessible | Hermes::RenderInterface::BufferUsageType::CopySource);
		VertexBuffer = Device->CreateBuffer(static_cast<Hermes::uint32>(SuzanneMesh->GetRequiredVertexBufferSize()), Hermes::RenderInterface::BufferUsageType::CopyDestination | Hermes::RenderInterface::BufferUsageType::VertexBuffer);
		IndexBuffer = Device->CreateBuffer(static_cast<Hermes::uint32>(SuzanneMesh->GetRequiredIndexBufferSize()), Hermes::RenderInterface::BufferUsageType::CopyDestination | Hermes::RenderInterface::BufferUsageType::IndexBuffer);
		auto TransferQueue = Device->GetQueue(Hermes::RenderInterface::QueueType::Transfer);
		auto TransferCommandBuffer = TransferQueue->CreateCommandBuffer(true);

		DepthBuffer = Device->CreateImage(
			ApplicationWindow->GetSize(), Hermes::RenderInterface::ImageUsageType::DepthStencilAttachment, 
			Hermes::RenderInterface::DataFormat::D32SignedFloat, 1, Hermes::RenderInterface::ImageLayout::Undefined);

		// Vertex data copy
		void* Dst = StagingBuffer->Map();
		memcpy(Dst, SuzanneMesh->GetRawVertexData(), SuzanneMesh->GetRequiredVertexBufferSize());
		StagingBuffer->Unmap();
		TransferCommandBuffer->BeginRecording();
		Hermes::RenderInterface::BufferCopyRegion Region = {};
		Region.SourceOffset = 0;
		Region.DestinationOffset = 0;
		Region.NumBytes = SuzanneMesh->GetRequiredVertexBufferSize();
		TransferCommandBuffer->CopyBuffer(*StagingBuffer, *VertexBuffer, { Region });
		Hermes::RenderInterface::BufferMemoryBarrier BufferBarrier = {};
		BufferBarrier.OperationsThatHaveToEndBefore = Hermes::RenderInterface::AccessType::TransferWrite;
		BufferBarrier.OperationsThatCanStartAfter = Hermes::RenderInterface::AccessType::MemoryRead;
		BufferBarrier.OldOwnerQueue = TransferQueue.get();
		BufferBarrier.NewOwnerQueue = Device->GetQueue(Hermes::RenderInterface::QueueType::Render).get();
		BufferBarrier.Offset = 0;
		BufferBarrier.NumBytes = VertexBuffer->GetSize();
		TransferCommandBuffer->InsertBufferMemoryBarrier(*VertexBuffer, BufferBarrier, Hermes::RenderInterface::PipelineStage::Transfer, Hermes::RenderInterface::PipelineStage::TopOfPipe);
		TransferCommandBuffer->EndRecording();
		TransferQueue->SubmitCommandBuffer(TransferCommandBuffer, {});
		TransferQueue->WaitForIdle();

		// Index data copy
		Dst = StagingBuffer->Map();
		memcpy(Dst, SuzanneMesh->GetRawIndexData(), SuzanneMesh->GetRequiredIndexBufferSize());
		StagingBuffer->Unmap();
		TransferCommandBuffer->BeginRecording();
		Region = {};
		Region.SourceOffset = 0;
		Region.DestinationOffset = 0;
		Region.NumBytes = SuzanneMesh->GetRequiredIndexBufferSize();
		TransferCommandBuffer->CopyBuffer(*StagingBuffer, *IndexBuffer, { Region });
		BufferBarrier.NumBytes = IndexBuffer->GetSize();
		TransferCommandBuffer->InsertBufferMemoryBarrier(*IndexBuffer, BufferBarrier, Hermes::RenderInterface::PipelineStage::Transfer, Hermes::RenderInterface::PipelineStage::TopOfPipe);
		TransferCommandBuffer->EndRecording();
		TransferQueue->SubmitCommandBuffer(TransferCommandBuffer, {});
		TransferQueue->WaitForIdle();

		// Image copy
		Dst = StagingBuffer->Map();
		memcpy(Dst, CheckerImage, ImageSizeInBytes);
		StagingBuffer->Unmap();
		TransferCommandBuffer->BeginRecording();
		Hermes::RenderInterface::ImageMemoryBarrier ImageBarrier = {};
		ImageBarrier.OldLayout = Hermes::RenderInterface::ImageLayout::Undefined;
		ImageBarrier.NewLayout = Hermes::RenderInterface::ImageLayout::TransferDestinationOptimal;
		ImageBarrier.OperationsThatCanStartAfter = Hermes::RenderInterface::AccessType::TransferWrite;
		ImageBarrier.OperationsThatHaveToEndBefore = Hermes::RenderInterface::AccessType::MemoryRead;
		TransferCommandBuffer->InsertImageMemoryBarrier(*Texture, ImageBarrier, Hermes::RenderInterface::PipelineStage::TopOfPipe, Hermes::RenderInterface::PipelineStage::Transfer);
		Hermes::RenderInterface::BufferToImageCopyRegion CopyRegion = {};
		CopyRegion.BufferOffset = 0;
		TransferCommandBuffer->CopyBufferToImage(*StagingBuffer, *Texture, Hermes::RenderInterface::ImageLayout::TransferDestinationOptimal, { CopyRegion });
		ImageBarrier.OldLayout = Hermes::RenderInterface::ImageLayout::TransferDestinationOptimal;
		ImageBarrier.NewLayout = Hermes::RenderInterface::ImageLayout::ShaderReadOnlyOptimal;
		ImageBarrier.OperationsThatHaveToEndBefore = Hermes::RenderInterface::AccessType::TransferWrite;
		ImageBarrier.OperationsThatCanStartAfter = Hermes::RenderInterface::AccessType::MemoryRead;
		ImageBarrier.OldOwnerQueue = TransferQueue.get();
		ImageBarrier.NewOwnerQueue = Device->GetQueue(Hermes::RenderInterface::QueueType::Render).get();
		TransferCommandBuffer->InsertImageMemoryBarrier(*Texture, ImageBarrier, Hermes::RenderInterface::PipelineStage::Transfer, Hermes::RenderInterface::PipelineStage::Transfer);
		TransferCommandBuffer->EndRecording();
		TransferQueue->SubmitCommandBuffer(TransferCommandBuffer, {});
		TransferQueue->WaitForIdle();

		// We now have to acquire ownership of all resources on render queue
		auto RenderQueue = Device->GetQueue(Hermes::RenderInterface::QueueType::Render);
		auto RenderCommandBuffer = RenderQueue->CreateCommandBuffer(true);
		RenderCommandBuffer->BeginRecording();
		BufferBarrier.NumBytes = VertexBuffer->GetSize();
		RenderCommandBuffer->InsertBufferMemoryBarrier(*VertexBuffer, BufferBarrier, Hermes::RenderInterface::PipelineStage::Transfer, Hermes::RenderInterface::PipelineStage::VertexInput);
		BufferBarrier.NumBytes = IndexBuffer->GetSize();
		RenderCommandBuffer->InsertBufferMemoryBarrier(*IndexBuffer, BufferBarrier, Hermes::RenderInterface::PipelineStage::Transfer, Hermes::RenderInterface::PipelineStage::VertexInput);
		RenderCommandBuffer->InsertImageMemoryBarrier(*Texture, ImageBarrier, Hermes::RenderInterface::PipelineStage::Transfer, Hermes::RenderInterface::PipelineStage::FragmentShader);
		Hermes::RenderInterface::ImageMemoryBarrier DepthBufferTransitionBarrier = {};
		DepthBufferTransitionBarrier.NewLayout = Hermes::RenderInterface::ImageLayout::DepthAttachmentOptimal;
		DepthBufferTransitionBarrier.OldLayout = Hermes::RenderInterface::ImageLayout::Undefined;
		DepthBufferTransitionBarrier.OperationsThatHaveToEndBefore = Hermes::RenderInterface::AccessType::MemoryWrite;
		DepthBufferTransitionBarrier.OperationsThatCanStartAfter = Hermes::RenderInterface::AccessType::MemoryRead | Hermes::RenderInterface::AccessType::MemoryWrite;
		RenderCommandBuffer->InsertImageMemoryBarrier(*DepthBuffer, DepthBufferTransitionBarrier, Hermes::RenderInterface::PipelineStage::TopOfPipe, Hermes::RenderInterface::PipelineStage::BottomOfPipe);
		RenderCommandBuffer->EndRecording();
		RenderQueue->SubmitCommandBuffer(RenderCommandBuffer, {});
		RenderQueue->WaitForIdle();
		

		VertexShader = Device->CreateShader(L"Shaders/Bin/basic_vert.glsl.spv", Hermes::RenderInterface::ShaderType::VertexShader);
		FragmentShader = Device->CreateShader(L"Shaders/Bin/basic_frag.glsl.spv", Hermes::RenderInterface::ShaderType::FragmentShader);

		Hermes::RenderInterface::RenderPassDescription Description = {};
		Description.ColorAttachments.push_back({});
		Description.ColorAttachments[0].LayoutAtStart = Hermes::RenderInterface::ImageLayout::ColorAttachmentOptimal;
		Description.ColorAttachments[0].LayoutAtEnd = Hermes::RenderInterface::ImageLayout::ReadyForPresentation;
		Description.ColorAttachments[0].Format = Swapchain->GetImageFormat();
		Description.ColorAttachments[0].LoadOp = Hermes::RenderInterface::AttachmentLoadOp::Clear;
		Description.ColorAttachments[0].StoreOp = Hermes::RenderInterface::AttachmentStoreOp::Store;
		Description.ColorAttachments[0].StencilLoadOp = Hermes::RenderInterface::AttachmentLoadOp::Undefined;
		Description.ColorAttachments[0].StencilStoreOp = Hermes::RenderInterface::AttachmentStoreOp::Undefined;
		Description.DepthAttachment = Hermes::RenderInterface::RenderPassAttachment{};
		Description.DepthAttachment.value().LayoutAtStart = Hermes::RenderInterface::ImageLayout::DepthStencilAttachmentOptimal;
		Description.DepthAttachment.value().LayoutAtEnd = Hermes::RenderInterface::ImageLayout::DepthStencilAttachmentOptimal;
		Description.DepthAttachment.value().Format = DepthBuffer->GetDataFormat();
		Description.DepthAttachment.value().LoadOp = Hermes::RenderInterface::AttachmentLoadOp::Clear;
		Description.DepthAttachment.value().StoreOp = Hermes::RenderInterface::AttachmentStoreOp::Undefined;
		Description.DepthAttachment.value().StencilLoadOp = Hermes::RenderInterface::AttachmentLoadOp::Undefined;
		Description.DepthAttachment.value().StencilStoreOp = Hermes::RenderInterface::AttachmentStoreOp::Undefined;
		RenderPass = Device->CreateRenderPass(Description);

		Hermes::RenderInterface::DescriptorBinding SamplerBinding = {};
		SamplerBinding.Index = 0;
		SamplerBinding.DescriptorCount = 1;
		SamplerBinding.Shader = Hermes::RenderInterface::ShaderType::FragmentShader;
		SamplerBinding.Type = Hermes::RenderInterface::DescriptorType::Sampler;
		Hermes::RenderInterface::DescriptorBinding TextureBinding = {};
		TextureBinding.Index = 1;
		TextureBinding.DescriptorCount = 1;
		TextureBinding.Shader = Hermes::RenderInterface::ShaderType::FragmentShader;
		TextureBinding.Type = Hermes::RenderInterface::DescriptorType::SampledImage;
		TextureDescriptorSetLayout = Device->CreateDescriptorSetLayout({ SamplerBinding, TextureBinding });
		std::vector<Hermes::RenderInterface::SubpoolDescription> Subpools =
		{
			{
				Hermes::RenderInterface::DescriptorType::Sampler,
				Swapchain->GetImageCount()
			},
			{
				
				Hermes::RenderInterface::DescriptorType::SampledImage,
				Swapchain->GetImageCount()
			}
		};
		auto TextureDescriptorSetPool = Device->CreateDescriptorSetPool(Swapchain->GetImageCount(), Subpools);
		
		Hermes::RenderInterface::PipelineDescription PipelineDesc = {};
		PipelineDesc.PushConstants =
			{
				{
					Hermes::RenderInterface::ShaderType::VertexShader,
					0,
					3 * sizeof(Hermes::Mat4)
				}
			};
		PipelineDesc.ShaderStages =
			{
				VertexShader,
				FragmentShader
			};
		PipelineDesc.DescriptorLayouts =
			{
				TextureDescriptorSetLayout
			};
		PipelineDesc.VertexInput.VertexBindings.push_back({});
		PipelineDesc.VertexInput.VertexBindings[0].Index = 0;
		PipelineDesc.VertexInput.VertexBindings[0].Stride = sizeof(float) * 8;
		PipelineDesc.VertexInput.VertexBindings[0].IsPerInstance = false;
		PipelineDesc.VertexInput.VertexAttributes.push_back({});
		PipelineDesc.VertexInput.VertexAttributes[0].BindingIndex = 0;
		PipelineDesc.VertexInput.VertexAttributes[0].Format = Hermes::RenderInterface::DataFormat::R32G32B32SignedFloat;
		PipelineDesc.VertexInput.VertexAttributes[0].Offset = 0;
		PipelineDesc.VertexInput.VertexAttributes[0].Location = 0;
		PipelineDesc.VertexInput.VertexAttributes.push_back({});
		PipelineDesc.VertexInput.VertexAttributes[1].BindingIndex = 0;
		PipelineDesc.VertexInput.VertexAttributes[1].Format = Hermes::RenderInterface::DataFormat::R32G32SignedFloat;
		PipelineDesc.VertexInput.VertexAttributes[1].Offset = 3 * sizeof(float);
		PipelineDesc.VertexInput.VertexAttributes[1].Location = 1;
		PipelineDesc.InputAssembler.Topology = Hermes::RenderInterface::TopologyType::TriangleList;
		PipelineDesc.Viewport.Origin = { 0, 0 };
		PipelineDesc.Viewport.Dimensions = Swapchain->GetSize();
		PipelineDesc.Rasterizer.Fill = Hermes::RenderInterface::FillMode::Fill;
		PipelineDesc.Rasterizer.Direction = Hermes::RenderInterface::FaceDirection::CounterClockwise;
		PipelineDesc.Rasterizer.Cull = Hermes::RenderInterface::CullMode::Back;
		PipelineDesc.DepthStencilStage.IsDepthTestEnabled = true;
		PipelineDesc.DepthStencilStage.IsDepthWriteEnabled = true;
		PipelineDesc.DepthStencilStage.ComparisonMode = Hermes::RenderInterface::ComparisonOperator::Less;

		Pipeline = Device->CreatePipeline(RenderPass, PipelineDesc);

		Hermes::RenderInterface::SamplerDescription SamplerDesc = {};
		SamplerDesc.CoordinateSystem = Hermes::RenderInterface::CoordinateSystem::Normalized;
		SamplerDesc.AddressingModeU = Hermes::RenderInterface::AddressingMode::Repeat;
		SamplerDesc.AddressingModeV = Hermes::RenderInterface::AddressingMode::Repeat;
		SamplerDesc.MinificationFilteringMode = Hermes::RenderInterface::FilteringMode::Linear;
		SamplerDesc.MagnificationFilteringMode = Hermes::RenderInterface::FilteringMode::Linear;
		TextureSampler = Device->CreateSampler(SamplerDesc);

		RenderTargets.reserve(Swapchain->GetImageCount());
		TextureDescriptorSets.reserve(Swapchain->GetImageCount());
		for (Hermes::uint32 Index = 0; Index < Swapchain->GetImageCount(); Index++)
		{
			RenderTargets.push_back(Device->CreateRenderTarget(RenderPass, {Swapchain->GetImage(Index), DepthBuffer}, Swapchain->GetSize()));
			TextureDescriptorSets.push_back(TextureDescriptorSetPool->CreateDescriptorSet(TextureDescriptorSetLayout));
			TextureDescriptorSets[Index]->UpdateWithSampler(0, 0, *TextureSampler);
			TextureDescriptorSets[Index]->UpdateWithImage(1, 0, *Texture, Hermes::RenderInterface::ImageLayout::ShaderReadOnlyOptimal);
		}

		GraphicsCommandBuffer = Device->GetQueue(Hermes::RenderInterface::QueueType::Render)->CreateCommandBuffer(true);
		GraphicsFence = Device->CreateFence(false);
		PresentationFence = Device->CreateFence(false);

		Hermes::Matrix<2, 2, int> Mat1;
		Hermes::Matrix<2, 3, int> Mat2;
		Mat1[0][0] = 11;
		Mat1[0][1] = 3;
		Mat1[1][0] = 7;
		Mat1[1][1] = 11;
		Mat2[0][0] = 8;
		Mat2[0][1] = 0;
		Mat2[0][2] = 1;
		Mat2[1][0] = 0;
		Mat2[1][1] = 3;
		Mat2[1][2] = 5;

		auto Mat = Mat1 * Mat2;
		
		return true;
	}

	void Run(float DeltaTime) override
	{
		if (SwapchainRecreated)
		{
			RenderTargets.clear();

			DepthBuffer = Device->CreateImage(
				ApplicationWindow->GetSize(), Hermes::RenderInterface::ImageUsageType::DepthStencilAttachment, 
				Hermes::RenderInterface::DataFormat::D32SignedFloat, 1, Hermes::RenderInterface::ImageLayout::Undefined);

			Hermes::RenderInterface::ImageMemoryBarrier DepthBufferTransitionBarrier = {};
			DepthBufferTransitionBarrier.NewLayout = Hermes::RenderInterface::ImageLayout::DepthAttachmentOptimal;
			DepthBufferTransitionBarrier.OldLayout = Hermes::RenderInterface::ImageLayout::Undefined;
			DepthBufferTransitionBarrier.OperationsThatHaveToEndBefore = Hermes::RenderInterface::AccessType::MemoryWrite;
			DepthBufferTransitionBarrier.OperationsThatCanStartAfter = Hermes::RenderInterface::AccessType::MemoryRead | Hermes::RenderInterface::AccessType::MemoryWrite;

			auto DepthBufferTransitionCommandBuffer = Device->GetQueue(Hermes::RenderInterface::QueueType::Render)->CreateCommandBuffer(true);
			DepthBufferTransitionCommandBuffer->BeginRecording();
			DepthBufferTransitionCommandBuffer->InsertImageMemoryBarrier(*DepthBuffer, DepthBufferTransitionBarrier, Hermes::RenderInterface::PipelineStage::BottomOfPipe, Hermes::RenderInterface::PipelineStage::TopOfPipe);
			DepthBufferTransitionCommandBuffer->EndRecording();
			Device->GetQueue(Hermes::RenderInterface::QueueType::Render)->SubmitCommandBuffer(DepthBufferTransitionCommandBuffer, {});
			Device->GetQueue(Hermes::RenderInterface::QueueType::Render)->WaitForIdle();

			Hermes::RenderInterface::RenderPassDescription Description = {};
			Description.ColorAttachments.push_back({});
			Description.ColorAttachments[0].LayoutAtStart = Hermes::RenderInterface::ImageLayout::ColorAttachmentOptimal;
			Description.ColorAttachments[0].LayoutAtEnd = Hermes::RenderInterface::ImageLayout::ReadyForPresentation;
			Description.ColorAttachments[0].Format = Swapchain->GetImageFormat();
			Description.ColorAttachments[0].LoadOp = Hermes::RenderInterface::AttachmentLoadOp::Clear;
			Description.ColorAttachments[0].StoreOp = Hermes::RenderInterface::AttachmentStoreOp::Store;
			Description.ColorAttachments[0].StencilLoadOp = Hermes::RenderInterface::AttachmentLoadOp::Undefined;
			Description.ColorAttachments[0].StencilStoreOp = Hermes::RenderInterface::AttachmentStoreOp::Undefined;
			Description.DepthAttachment = Hermes::RenderInterface::RenderPassAttachment{};
			Description.DepthAttachment.value().LayoutAtStart = Hermes::RenderInterface::ImageLayout::DepthStencilAttachmentOptimal;
			Description.DepthAttachment.value().LayoutAtEnd = Hermes::RenderInterface::ImageLayout::DepthStencilAttachmentOptimal;
			Description.DepthAttachment.value().Format = DepthBuffer->GetDataFormat();
			Description.DepthAttachment.value().LoadOp = Hermes::RenderInterface::AttachmentLoadOp::Clear;
			Description.DepthAttachment.value().StoreOp = Hermes::RenderInterface::AttachmentStoreOp::Undefined;
			Description.DepthAttachment.value().StencilLoadOp = Hermes::RenderInterface::AttachmentLoadOp::Undefined;
			Description.DepthAttachment.value().StencilStoreOp = Hermes::RenderInterface::AttachmentStoreOp::Undefined;
			RenderPass = Device->CreateRenderPass(Description);

			Hermes::RenderInterface::PipelineDescription PipelineDesc = {};
			PipelineDesc.PushConstants =
				{
					{
						Hermes::RenderInterface::ShaderType::VertexShader,
						0,
						3 * sizeof(Hermes::Mat4)
					}
				};
			PipelineDesc.ShaderStages =
				{
					VertexShader,
					FragmentShader
				};
			PipelineDesc.DescriptorLayouts =
				{
					TextureDescriptorSetLayout
				};
			PipelineDesc.VertexInput.VertexBindings.push_back({});
			PipelineDesc.VertexInput.VertexBindings[0].Index = 0;
			PipelineDesc.VertexInput.VertexBindings[0].Stride = sizeof(float) * 8;
			PipelineDesc.VertexInput.VertexBindings[0].IsPerInstance = false;
			PipelineDesc.VertexInput.VertexAttributes.push_back({});
			PipelineDesc.VertexInput.VertexAttributes[0].BindingIndex = 0;
			PipelineDesc.VertexInput.VertexAttributes[0].Format = Hermes::RenderInterface::DataFormat::R32G32B32SignedFloat;
			PipelineDesc.VertexInput.VertexAttributes[0].Offset = 0;
			PipelineDesc.VertexInput.VertexAttributes[0].Location = 0;
			PipelineDesc.VertexInput.VertexAttributes.push_back({});
			PipelineDesc.VertexInput.VertexAttributes[1].BindingIndex = 0;
			PipelineDesc.VertexInput.VertexAttributes[1].Format = Hermes::RenderInterface::DataFormat::R32G32SignedFloat;
			PipelineDesc.VertexInput.VertexAttributes[1].Offset = 3 * sizeof(float);
			PipelineDesc.VertexInput.VertexAttributes[1].Location = 1;
			PipelineDesc.InputAssembler.Topology = Hermes::RenderInterface::TopologyType::TriangleList;
			PipelineDesc.Viewport.Origin = { 0, 0 };
			PipelineDesc.Viewport.Dimensions = Swapchain->GetSize();
			PipelineDesc.Rasterizer.Fill = Hermes::RenderInterface::FillMode::Fill;
			PipelineDesc.Rasterizer.Direction = Hermes::RenderInterface::FaceDirection::CounterClockwise;
			PipelineDesc.Rasterizer.Cull = Hermes::RenderInterface::CullMode::Back;
			PipelineDesc.DepthStencilStage.IsDepthTestEnabled = true;
			PipelineDesc.DepthStencilStage.IsDepthWriteEnabled = true;
			PipelineDesc.DepthStencilStage.ComparisonMode = Hermes::RenderInterface::ComparisonOperator::Less;

			Pipeline = Device->CreatePipeline(RenderPass, PipelineDesc);

			RenderTargets.reserve(Swapchain->GetImageCount());
			TextureDescriptorSets.reserve(Swapchain->GetImageCount());
			for (Hermes::uint32 Index = 0; Index < Swapchain->GetImageCount(); Index++)
			{
				RenderTargets.push_back(Device->CreateRenderTarget(RenderPass, {Swapchain->GetImage(Index), DepthBuffer}, Swapchain->GetSize()));
				TextureDescriptorSets[Index]->UpdateWithSampler(0, 0, *TextureSampler);
				TextureDescriptorSets[Index]->UpdateWithImage(1, 0, *Texture, Hermes::RenderInterface::ImageLayout::ShaderReadOnlyOptimal);
			}
		}

		SwapchainRecreated = false;
		auto NewIndex = Swapchain->AcquireImage(UINT64_MAX, *PresentationFence, SwapchainRecreated);
		if (SwapchainRecreated)
			return; // No sense in rendering frame as it won't be able to be presented. Skip to next iteration
		PresentationFence->Wait(UINT64_MAX);
		HERMES_ASSERT(NewIndex.has_value());
		Hermes::uint32 ImageIndex = NewIndex.value();

		TimeSinceStart += DeltaTime;
		const auto& InputEngine = Hermes::GGameLoop->GetInputEngine();

		static constexpr float CameraRotationSpeed = 150.0f;
		Hermes::Vec2 MouseDelta = InputEngine.GetDeltaMousePosition();
		MouseDelta = MouseDelta.SafeNormalize() * DeltaTime * CameraRotationSpeed * Hermes::Vec2{ 1.0f, -1.0f };
		CameraPitch = Hermes::Math::Clamp(-85.0f, 85.0f, CameraPitch + MouseDelta.Y);
		CameraYaw += MouseDelta.X;
		CameraYaw = fmod(CameraYaw, 360.0f);
		if (CameraYaw > 180.0f)
			CameraYaw = 360.0f - CameraYaw;
		if (CameraYaw < -180.0f)
			CameraYaw = 360.0f + CameraYaw;
		Hermes::Vec3 CameraForward;
		CameraForward.X = Hermes::Math::Sin(Hermes::Math::Radians(CameraYaw));
		CameraForward.Y = Hermes::Math::Sin(Hermes::Math::Radians(CameraPitch));
		CameraForward.Z = Hermes::Math::Cos(Hermes::Math::Radians(CameraYaw)) * Hermes::Math::Cos(Hermes::Math::Radians(CameraPitch));
		CameraForward.Normalize();
		Hermes::Vec3 GlobalUp = { 0.0f, 1.0f, 0.0f };
		Hermes::Vec3 CameraRight = (GlobalUp ^ CameraForward).Normalize();
		Hermes::Vec3 CameraUp = (CameraForward ^ CameraRight).Normalize();
		
		Hermes::Vec3 DeltaCameraPosition = {};
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::W))
			DeltaCameraPosition += CameraForward;
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::S))
			DeltaCameraPosition -= CameraForward;
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::D))
			DeltaCameraPosition += CameraRight;
		if (InputEngine.IsKeyPressed(Hermes::KeyCode::A))
			DeltaCameraPosition -= CameraRight;
		DeltaCameraPosition.SafeNormalize();
		DeltaCameraPosition *= DeltaTime * 10.0f;
		CameraPos += DeltaCameraPosition;

		struct PushConstant
		{
			Hermes::Mat4 ModelMatrix;
			Hermes::Mat4 ViewMatrix;
			Hermes::Mat4 ProjectionMatrix;
		} PushConstants;
		PushConstants.ProjectionMatrix = Hermes::Mat4::Perspective(Hermes::Math::Radians(60.0f), 1280.0f / 720.0f, 0.01f, 100.0f);
		PushConstants.ViewMatrix = Hermes::Mat4::LookAt(CameraPos, CameraForward, CameraUp);
		PushConstants.ModelMatrix = Hermes::Mat4::Translation(Hermes::Vec3{0.0f, 0.0f, 10.0f});
		
		GraphicsCommandBuffer->BeginRecording();
		// WARNING : always make sure to clear depth buffer with 1.0 in all channels because we use less comparision operator
		GraphicsCommandBuffer->BeginRenderPass(RenderPass, RenderTargets[ImageIndex], { {1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } });
		GraphicsCommandBuffer->BindPipeline(Pipeline);
		GraphicsCommandBuffer->BindVertexBuffer(*VertexBuffer);
		GraphicsCommandBuffer->BindIndexBuffer(*IndexBuffer, Hermes::RenderInterface::IndexSize::Uint32);
		GraphicsCommandBuffer->BindDescriptorSet(*TextureDescriptorSets[ImageIndex], *Pipeline, 0);
		GraphicsCommandBuffer->UploadPushConstants(*Pipeline, Hermes::RenderInterface::ShaderType::VertexShader, &PushConstants, sizeof(PushConstants), 0);
		GraphicsCommandBuffer->DrawIndexed(DrawIndexCount, 1, 0, 0, 0);
		GraphicsCommandBuffer->EndRenderPass();
		GraphicsCommandBuffer->EndRecording();
		Device->GetQueue(Hermes::RenderInterface::QueueType::Render)->SubmitCommandBuffer(GraphicsCommandBuffer, GraphicsFence);
		Device->GetQueue(Hermes::RenderInterface::QueueType::Render)->WaitForIdle();

		GraphicsFence->Reset();
		Swapchain->Present(ImageIndex, SwapchainRecreated);
		Device->GetQueue(Hermes::RenderInterface::QueueType::Presentation)->WaitForIdle();
		PresentationFence->Reset();
	}

	void Shutdown() override
	{
	}

private:
	std::shared_ptr<const Hermes::IPlatformWindow> ApplicationWindow;

	std::weak_ptr<Hermes::EventQueue> WindowMessageQueue;
	
	std::shared_ptr<Hermes::RenderInterface::Device> Device;
	std::shared_ptr<Hermes::RenderInterface::Swapchain> Swapchain;
	std::shared_ptr<Hermes::RenderInterface::RenderPass> RenderPass;
	std::shared_ptr<Hermes::RenderInterface::Pipeline> Pipeline;
	std::shared_ptr<Hermes::RenderInterface::Buffer> VertexBuffer, IndexBuffer;
	std::shared_ptr<Hermes::RenderInterface::Image> DepthBuffer;
	std::shared_ptr<Hermes::RenderInterface::RenderTarget> DepthBufferRenderTarget;
	std::vector<std::shared_ptr<Hermes::RenderInterface::RenderTarget>> RenderTargets;
	std::vector<std::shared_ptr<Hermes::RenderInterface::DescriptorSet>> TextureDescriptorSets;
	std::shared_ptr<Hermes::RenderInterface::DescriptorSetLayout> TextureDescriptorSetLayout;
	std::shared_ptr<Hermes::RenderInterface::CommandBuffer> GraphicsCommandBuffer;
	std::shared_ptr<Hermes::RenderInterface::Fence> GraphicsFence, PresentationFence;
	std::shared_ptr<Hermes::RenderInterface::Sampler> TextureSampler;
	std::shared_ptr<Hermes::RenderInterface::Image> Texture;
	std::shared_ptr<Hermes::RenderInterface::Shader> VertexShader, FragmentShader;

	float TimeSinceStart = 0.0f;
	Hermes::Vec3 CameraPos = {0.0f, 0.0f, -0.5f};
	float CameraPitch = 0.0f, CameraYaw = 0.0f;

	Hermes::uint32 DrawIndexCount = 0;

	bool SwapchainRecreated = false;
};

extern "C" _declspec(dllexport) Hermes::IApplication* CreateApplicationInstance()
{
	auto App = new SandboxApp;

	return App;
}

#endif
