#include "Math/Common.h"
#include "Math/Vector.h"
#include "Platform/GenericPlatform/PlatformFile.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
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

void WindowEventHandler(const Hermes::IEvent& Event)
{
	HERMES_LOG_INFO(L"Received window event: %s", Event.ToString().c_str());
}

class SandboxApp : public Hermes::IApplication
{
public:
	bool Init() override
	{
		ApplicationWindow = Hermes::GGameLoop->GetWindow();
		
		auto RenderInstance = Hermes::RenderInterface::Instance::CreateRenderInterfaceInstance(*ApplicationWindow);
		auto Devices = RenderInstance->EnumerateAvailableDevices();
		auto PhysicalDevice = RenderInstance->GetPhysicalDevice(Devices[0].InternalIndex);
		Device = PhysicalDevice->CreateDevice();
		Swapchain = Device->CreateSwapchain({ 1280, 720 }, 3);

		Hermes::Vec3 VertexData[] =
		{
			{ -0.5f, -0.5f, 0.0f },
			{  0.5f, -0.5f, 0.0f },
			{  0.5f,  0.5f, 0.0f },
			{  0.0f,  0.8f, 0.0f},
			{ -0.5f,  0.5f, 0.0f },
		};

		// Square is drawn indexed, triangle - only with vertex buffer(vertices 2, 3, 4)
		Hermes::uint32 IndexData[] =
		{
			0, 1, 2,
			2, 4, 0
		};

		struct UniformBufferData
		{
			Hermes::Vec3 Color = { 1.0f, 0.5f, 0.0f };
		} UniformData;

		Hermes::Vec2ui ImageSize;
		auto CheckerImage = LoadTGA(L"checker.tga", ImageSize);
		auto Image = Device->CreateImage(
			ImageSize, Hermes::RenderInterface::ImageUsageType::CopyDestination | Hermes::RenderInterface::ImageUsageType::Sampled,
			Hermes::RenderInterface::DataFormat::R8G8B8A8SRGB, 1, Hermes::RenderInterface::ImageLayout::Undefined);
		Hermes::uint32 ImageSizeInBytes = ImageSize.X * ImageSize.Y * 4;

		Hermes::uint32 StagingBufferSize = Hermes::Math::Max(
			Hermes::Math::Max(
				Hermes::Math::Max(
					(Hermes::uint32)sizeof(VertexData),
					(Hermes::uint32)sizeof(IndexData)),
				(Hermes::uint32)sizeof(UniformData)),
			ImageSizeInBytes);
		auto StagingBuffer = Device->CreateBuffer(StagingBufferSize, Hermes::RenderInterface::BufferUsageType::CPUAccessible | Hermes::RenderInterface::BufferUsageType::CopySource);
		VertexBuffer = Device->CreateBuffer(sizeof(VertexData), Hermes::RenderInterface::BufferUsageType::CopyDestination | Hermes::RenderInterface::BufferUsageType::VertexBuffer);
		IndexBuffer = Device->CreateBuffer(sizeof(IndexData), Hermes::RenderInterface::BufferUsageType::CopyDestination | Hermes::RenderInterface::BufferUsageType::IndexBuffer);
		UniformBuffer = Device->CreateBuffer(sizeof(UniformData), Hermes::RenderInterface::BufferUsageType::CopyDestination | Hermes::RenderInterface::BufferUsageType::UniformBuffer);
		auto TransferQueue = Device->GetQueue(Hermes::RenderInterface::QueueType::Transfer);
		auto TransferCommandBuffer = TransferQueue->CreateCommandBuffer(true);

		// Vertex data copy
		void* Dst = StagingBuffer->Map();
		memcpy(Dst, VertexData, sizeof(VertexData));
		StagingBuffer->Unmap();
		TransferCommandBuffer->BeginRecording();
		Hermes::RenderInterface::BufferCopyRegion Region = {};
		Region.SourceOffset = 0;
		Region.DestinationOffset = 0;
		Region.NumBytes = sizeof(VertexData);
		TransferCommandBuffer->CopyBuffer(*StagingBuffer, *VertexBuffer, { Region });
		TransferCommandBuffer->EndRecording();
		TransferQueue->SubmitCommandBuffer(TransferCommandBuffer, {});
		Device->WaitForIdle(); // TODO : add Queue::WaitForIdle()

		// Index data copy
		Dst = StagingBuffer->Map();
		memcpy(Dst, IndexData, sizeof(IndexData));
		StagingBuffer->Unmap();
		TransferCommandBuffer->BeginRecording();
		Region = {};
		Region.SourceOffset = 0;
		Region.DestinationOffset = 0;
		Region.NumBytes = sizeof(IndexData);
		TransferCommandBuffer->CopyBuffer(*StagingBuffer, *IndexBuffer, { Region });
		TransferCommandBuffer->EndRecording();
		TransferQueue->SubmitCommandBuffer(TransferCommandBuffer, {});
		Device->WaitForIdle();

		// Uniform data copy
		Dst = StagingBuffer->Map();
		memcpy(Dst, &UniformData, sizeof(UniformData));
		StagingBuffer->Unmap();
		TransferCommandBuffer->BeginRecording();
		Region = {};
		Region.SourceOffset = 0;
		Region.DestinationOffset = 0;
		Region.NumBytes = sizeof(UniformData);
		TransferCommandBuffer->CopyBuffer(*StagingBuffer, *UniformBuffer, { Region });
		TransferCommandBuffer->EndRecording();
		TransferQueue->SubmitCommandBuffer(TransferCommandBuffer, {});
		Device->WaitForIdle();

		// Image copy
		Dst = StagingBuffer->Map();
		memcpy(Dst, CheckerImage, ImageSizeInBytes);
		StagingBuffer->Unmap();
		TransferCommandBuffer->BeginRecording();
		Hermes::RenderInterface::ImageMemoryBarrier Barrier = {};
		Barrier.OldLayout = Hermes::RenderInterface::ImageLayout::Undefined;
		Barrier.NewLayout = Hermes::RenderInterface::ImageLayout::TransferDestinationOptimal;
		Barrier.OperationsThatCanStartAfter = Hermes::RenderInterface::AccessType::TransferWrite;
		Barrier.OperationsThatHaveToEndBefore = Hermes::RenderInterface::AccessType::MemoryRead;
		TransferCommandBuffer->InsertImageMemoryBarrier(*Image, Barrier, Hermes::RenderInterface::PipelineStage::TopOfPipe, Hermes::RenderInterface::PipelineStage::Transfer);
		Hermes::RenderInterface::BufferToImageCopyRegion CopyRegion = {};
		CopyRegion.BufferOffset = 0;
		TransferCommandBuffer->CopyBufferToImage(*StagingBuffer, *Image, Hermes::RenderInterface::ImageLayout::TransferDestinationOptimal, { CopyRegion });
		Barrier.OldLayout = Hermes::RenderInterface::ImageLayout::TransferDestinationOptimal;
		Barrier.NewLayout = Hermes::RenderInterface::ImageLayout::ShaderReadOnlyOptimal;
		Barrier.OperationsThatHaveToEndBefore = Hermes::RenderInterface::AccessType::TransferWrite;
		Barrier.OperationsThatCanStartAfter = Hermes::RenderInterface::AccessType::MemoryRead;
		Barrier.OldOwnerQueue = TransferQueue.get();
		Barrier.NewOwnerQueue = Device->GetQueue(Hermes::RenderInterface::QueueType::Render).get();
		TransferCommandBuffer->InsertImageMemoryBarrier(*Image, Barrier, Hermes::RenderInterface::PipelineStage::Transfer, Hermes::RenderInterface::PipelineStage::Transfer);
		TransferCommandBuffer->EndRecording();
		TransferQueue->SubmitCommandBuffer(TransferCommandBuffer, {});
		Device->WaitForIdle();
		free(CheckerImage);

		auto VertexShader = Device->CreateShader(L"Shaders/Bin/basic_vert.glsl.spv", Hermes::RenderInterface::ShaderType::VertexShader);
		auto FragmentShader = Device->CreateShader(L"Shaders/Bin/basic_frag.glsl.spv", Hermes::RenderInterface::ShaderType::FragmentShader);

		Hermes::RenderInterface::RenderPassDescription Description = {};
		Description.ColorAttachments.push_back({});
		Description.ColorAttachments[0].LayoutAtStart = Hermes::RenderInterface::ImageLayout::ColorAttachmentOptimal;
		Description.ColorAttachments[0].LayoutAtEnd = Hermes::RenderInterface::ImageLayout::ReadyForPresentation;
		Description.ColorAttachments[0].Format = Swapchain->GetImageFormat();
		Description.ColorAttachments[0].LoadOp = Hermes::RenderInterface::AttachmentLoadOp::Clear;
		Description.ColorAttachments[0].StoreOp = Hermes::RenderInterface::AttachmentStoreOp::Store;
		Description.ColorAttachments[0].StencilLoadOp = Hermes::RenderInterface::AttachmentLoadOp::Undefined;
		Description.ColorAttachments[0].StencilStoreOp = Hermes::RenderInterface::AttachmentStoreOp::Undefined;
		RenderPass = Device->CreateRenderPass(Description);

		Hermes::RenderInterface::DescriptorBinding Binding = {};
		Binding.Index = 0;
		Binding.DescriptorCount = 1;
		Binding.Shader = Hermes::RenderInterface::ShaderType::FragmentShader;
		auto DescriptorSetLayout = Device->CreateDescriptorSetLayout({ Binding });
		auto DescriptorSetPool = Device->CreateDescriptorSetPool(Swapchain->GetImageCount());
		
		Hermes::RenderInterface::PipelineDescription PipelineDesc = {};
		PipelineDesc.ShaderStages =
			{
				VertexShader,
				FragmentShader
			};
		PipelineDesc.DescriptorLayouts =
			{
				DescriptorSetLayout
			};
		PipelineDesc.VertexInput.VertexBindings.push_back({});
		PipelineDesc.VertexInput.VertexBindings[0].Index = 0;
		PipelineDesc.VertexInput.VertexBindings[0].Stride = sizeof(Hermes::Vec3);
		PipelineDesc.VertexInput.VertexBindings[0].IsPerInstance = false;
		PipelineDesc.VertexInput.VertexAttributes.push_back({});
		PipelineDesc.VertexInput.VertexAttributes[0].BindingIndex = 0;
		PipelineDesc.VertexInput.VertexAttributes[0].Format = Hermes::RenderInterface::DataFormat::R32G32B32SignedFloat;
		PipelineDesc.VertexInput.VertexAttributes[0].Offset = 0;
		PipelineDesc.VertexInput.VertexAttributes[0].Location = 0;
		PipelineDesc.InputAssembler.Topology = Hermes::RenderInterface::TopologyType::TriangleList;
		PipelineDesc.Viewport.Origin = { 0, 0 };
		PipelineDesc.Viewport.Dimensions = Swapchain->GetSize();
		PipelineDesc.Rasterizer.Fill = Hermes::RenderInterface::FillMode::Fill;
		PipelineDesc.Rasterizer.Direction = Hermes::RenderInterface::FaceDirection::Clockwise;
		PipelineDesc.Rasterizer.Cull = Hermes::RenderInterface::CullMode::Back;
		PipelineDesc.DepthStencilStage.IsDepthTestEnabled = false;
		PipelineDesc.DepthStencilStage.IsDepthWriteEnabled = false;

		Pipeline = Device->CreatePipeline(RenderPass, PipelineDesc);

		RenderTargets.reserve(Swapchain->GetImageCount());
		DescriptorSets.reserve(Swapchain->GetImageCount());
		for (Hermes::uint32 Index = 0; Index < Swapchain->GetImageCount(); Index++)
		{
			RenderTargets.push_back(Device->CreateRenderTarget(RenderPass, {Swapchain->GetImage(Index)}, Swapchain->GetSize()));
			DescriptorSets.push_back(DescriptorSetPool->CreateDescriptorSet(DescriptorSetLayout));
			DescriptorSets[Index]->Update(0, 0, *UniformBuffer, 0, (Hermes::uint32)UniformBuffer->GetSize());
		}

		GraphicsCommandBuffer = Device->GetQueue(Hermes::RenderInterface::QueueType::Render)->CreateCommandBuffer(true);
		GraphicsFence = Device->CreateFence(false);
		PresentationFence = Device->CreateFence(false);
		
		return true;
	}

	void Run(float) override
	{
		auto NewIndex = Swapchain->AcquireImage(UINT64_MAX, *PresentationFence);
		PresentationFence->Wait(UINT64_MAX);
		HERMES_ASSERT(NewIndex.has_value());
		Hermes::uint32 ImageIndex = NewIndex.value();
		
		GraphicsCommandBuffer->BeginRecording();
		GraphicsCommandBuffer->BeginRenderPass(RenderPass, RenderTargets[ImageIndex], { {1.0f, 1.0f, 0.0f, 1.0f } });
		GraphicsCommandBuffer->BindPipeline(Pipeline);
		GraphicsCommandBuffer->BindVertexBuffer(*VertexBuffer);
		GraphicsCommandBuffer->BindIndexBuffer(*IndexBuffer, Hermes::RenderInterface::IndexSize::Uint32);
		GraphicsCommandBuffer->BindDescriptorSet(*DescriptorSets[ImageIndex], *Pipeline, 0);
		GraphicsCommandBuffer->DrawIndexed(6, 1, 0, 0, 0);
		GraphicsCommandBuffer->Draw(3, 1, 2, 0);
		GraphicsCommandBuffer->EndRenderPass();
		GraphicsCommandBuffer->EndRecording();
		Device->GetQueue(Hermes::RenderInterface::QueueType::Render)->SubmitCommandBuffer(GraphicsCommandBuffer, GraphicsFence);
		GraphicsFence->Wait(UINT64_MAX);

		GraphicsFence->Reset();
		Swapchain->Present(ImageIndex);
		Device->WaitForIdle(); // TODO : fix it!
		PresentationFence->Reset();
	}

	void Shutdown() override
	{
		
	}

	// NOTE : test-only function, will be removed soon
	// NOTE : call free() after you're done with image
	static void* LoadTGA(const Hermes::String& Path, Hermes::Vec2ui& Dimensions)
	{
		auto File = Hermes::PlatformFilesystem::OpenFile(Path, Hermes::IPlatformFile::FileAccessMode::Read, Hermes::IPlatformFile::FileOpenMode::OpenExisting);
		if (!File)
			return nullptr;

		bool IsNewFormat;
		if (File->Size() < 26)
		{
			IsNewFormat = false;
		}
		else
		{
#pragma pack(push, 1)
			struct TGAFooter
			{
				Hermes::uint32 ExtensionAreaOffset;
				Hermes::uint32 DeveloperDirectoryOffset;
				Hermes::uint8 Signature[18];
			} Footer = {};
#pragma pack(pop)
			File->Seek(File->Size() - sizeof(Footer));
			if (!File->Read(reinterpret_cast<Hermes::uint8*>(&Footer), sizeof(Footer)))
			{
				return nullptr;
			}
			if (strncmp(reinterpret_cast<const char*>(Footer.Signature), "TRUEVISION-XFILE.", sizeof(Footer.Signature)) == 0)
				IsNewFormat = true;
			else
				IsNewFormat = false;
		}

#pragma pack(push, 1)
		struct TGAHeader
		{
			Hermes::uint8 IDLength;
			Hermes::uint8 ColorMapType;
			Hermes::uint8 ImageType;
			Hermes::uint8 ColorMapSpecificationSkipped[5]; // We're not loading images with color map, so we'll just skip this field and add 5 padding bytes instead
			struct ImageSpecification
			{
				Hermes::uint16 XOrigin;
				Hermes::uint16 YOrigin;
				Hermes::uint16 Width;
				Hermes::uint16 Height;
				Hermes::uint8  PixelDepth;
				Hermes::uint8  ImageDescriptor;
			} Specification;
		} Header = {};
#pragma pack(pop)
		File->Seek(0);
		if (!File->Read(reinterpret_cast<Hermes::uint8*>(&Header), sizeof(Header)))
		{
			return nullptr;
		}

		if (Header.Specification.PixelDepth != 32) // We're loading only 32 bit-per-pixel(RGBA) images for now
			return nullptr;
		if (Header.Specification.XOrigin != 0 || Header.Specification.YOrigin != 0)
			return nullptr;
		if (Header.ColorMapType != 0) // We don't want to load images that use color map
			return nullptr;
		if (Header.ImageType != 2) // Only uncompressed true-color images are loaded
			return nullptr;
		if ((Header.Specification.ImageDescriptor & 0x0F) != 8) // If not 8 bits per channel
			return nullptr;
		if ((Header.Specification.ImageDescriptor & 0x30) >> 4 != 0) // If not left-to-right top-to-bottom
			return nullptr;
		if ((Header.Specification.ImageDescriptor & 0xC0) != 0) // These fields are reserved and must be zero
			return nullptr;

		size_t ImageSize = static_cast<size_t>(Header.Specification.Width) * Header.Specification.Height;
		size_t BytesUsed = ImageSize * Header.Specification.PixelDepth / 8;
		void* Result = malloc(BytesUsed);
		if (!Result)
			return Result;

		if (!File->Read(static_cast<Hermes::uint8*>(Result), BytesUsed))
		{
			free(Result);
			return nullptr;
		}
		Dimensions.X = Header.Specification.Width;
		Dimensions.Y = Header.Specification.Height;
		return Result;
	}

private:
	std::shared_ptr<const Hermes::IPlatformWindow> ApplicationWindow;

	std::weak_ptr<Hermes::EventQueue> WindowMessageQueue;
	
	std::shared_ptr<Hermes::RenderInterface::Device> Device;
	std::shared_ptr<Hermes::RenderInterface::Swapchain> Swapchain;
	std::shared_ptr<Hermes::RenderInterface::RenderPass> RenderPass;
	std::shared_ptr<Hermes::RenderInterface::Pipeline> Pipeline;
	std::shared_ptr<Hermes::RenderInterface::Buffer> VertexBuffer, IndexBuffer, UniformBuffer;
	std::vector<std::shared_ptr<Hermes::RenderInterface::RenderTarget>> RenderTargets;
	std::vector<std::shared_ptr<Hermes::RenderInterface::DescriptorSet>> DescriptorSets;
	std::shared_ptr<Hermes::RenderInterface::CommandBuffer> GraphicsCommandBuffer;
	std::shared_ptr<Hermes::RenderInterface::Fence> GraphicsFence, PresentationFence;
};

extern "C" _declspec(dllexport) Hermes::IApplication* CreateApplicationInstance()
{
	auto App = new SandboxApp;

	return App;
}

#endif
