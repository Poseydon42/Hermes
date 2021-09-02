#include "Math/Common.h"
#include "Math/Vector.h"
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
		auto RenderInstance = Hermes::RenderInterface::Instance::CreateRenderInterfaceInstance(*Hermes::GGameLoop->GetWindow());
		auto Devices = RenderInstance->EnumerateAvailableDevices();
		auto PhysicalDevice = RenderInstance->GetPhysicalDevice(Devices[0].InternalIndex);
		auto Device = PhysicalDevice->CreateDevice();
		auto Swapchain = Device->CreateSwapchain({ 1280, 720 }, 3);

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

		Hermes::uint32 StagingBufferSize = Hermes::Math::Max((Hermes::uint32)sizeof(VertexData), (Hermes::uint32)sizeof(IndexData));
		auto StagingBuffer = Device->CreateBuffer(StagingBufferSize, Hermes::RenderInterface::ResourceUsageType::CPUAccessible | Hermes::RenderInterface::ResourceUsageType::CopySource);
		auto VertexBuffer = Device->CreateBuffer(sizeof(VertexData), Hermes::RenderInterface::ResourceUsageType::CopyDestination | Hermes::RenderInterface::ResourceUsageType::VertexBuffer);
		auto IndexBuffer = Device->CreateBuffer(sizeof(IndexData), Hermes::RenderInterface::ResourceUsageType::CopyDestination | Hermes::RenderInterface::ResourceUsageType::IndexBuffer);
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
		TransferCommandBuffer->CopyBuffer(StagingBuffer, VertexBuffer, { Region });
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
		TransferCommandBuffer->CopyBuffer(StagingBuffer, IndexBuffer, { Region });
		TransferCommandBuffer->EndRecording();
		TransferQueue->SubmitCommandBuffer(TransferCommandBuffer, {});
		Device->WaitForIdle();

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
		auto RenderPass = Device->CreateRenderPass(Description);

		Hermes::RenderInterface::PipelineDescription PipelineDesc = {};
		PipelineDesc.ShaderStages =
			{
				VertexShader,
				FragmentShader
			};
		PipelineDesc.VertexInput.VertexBindings.clear();
		PipelineDesc.VertexInput.VertexAttributes.clear();
		PipelineDesc.InputAssembler.Topology = Hermes::RenderInterface::TopologyType::TriangleList;
		PipelineDesc.Viewport.Origin = { 0, 0 };
		PipelineDesc.Viewport.Dimensions = Swapchain->GetSize();
		PipelineDesc.Rasterizer.Fill = Hermes::RenderInterface::FillMode::Fill;
		PipelineDesc.Rasterizer.Direction = Hermes::RenderInterface::FaceDirection::Clockwise;
		PipelineDesc.Rasterizer.Cull = Hermes::RenderInterface::CullMode::Back;
		PipelineDesc.DepthStencilStage.IsDepthTestEnabled = false;
		PipelineDesc.DepthStencilStage.IsDepthWriteEnabled = false;

		auto Pipeline = Device->CreatePipeline(RenderPass, PipelineDesc);

		auto RenderTarget = Device->CreateRenderTarget(RenderPass, {Swapchain->GetImage(0)}, Swapchain->GetSize());

		auto GraphicsCommandBuffer = Device->GetQueue(Hermes::RenderInterface::QueueType::Render)->CreateCommandBuffer(true);
		auto GraphicsFence = Device->CreateFence(false);
		GraphicsCommandBuffer->BeginRecording();
		GraphicsCommandBuffer->BeginRenderPass(RenderPass, RenderTarget, { {1.0f, 1.0f, 0.0f, 1.0f } });
		GraphicsCommandBuffer->BindPipeline(Pipeline);
		GraphicsCommandBuffer->BindVertexBuffer(*VertexBuffer);
		GraphicsCommandBuffer->BindIndexBuffer(*IndexBuffer, Hermes::RenderInterface::IndexSize::Uint32);
		GraphicsCommandBuffer->DrawIndexed(6, 1, 0, 0, 0);
		GraphicsCommandBuffer->Draw(3, 1, 2, 0);
		GraphicsCommandBuffer->EndRenderPass();
		GraphicsCommandBuffer->EndRecording();
		Device->GetQueue(Hermes::RenderInterface::QueueType::Render)->SubmitCommandBuffer(GraphicsCommandBuffer, GraphicsFence);
		GraphicsFence->Wait(UINT64_MAX);
		
		return true;
	}

	void Run(float) override
	{
	}

	void Shutdown() override
	{
		
	}

private:
	std::shared_ptr<Hermes::IPlatformWindow> ApplicationWindow;

	std::weak_ptr<Hermes::EventQueue> WindowMessageQueue;
};

extern "C" _declspec(dllexport) Hermes::IApplication* CreateApplicationInstance()
{
	auto App = new SandboxApp;

	return App;
}

#endif
