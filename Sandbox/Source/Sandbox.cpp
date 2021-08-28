#ifdef HERMES_PLATFORM_WINDOWS

#include "Core/Core.h"
#include "Core/Application/Application.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
#include "Platform/GenericPlatform/PlatformLibrary.h"
#include "RenderInterface/GenericRenderInterface/Resource.h"
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
		std::shared_ptr<Hermes::RenderInterface::PhysicalDevice> PhysicalDevice = RenderInstance->GetPhysicalDevice(Devices[0].InternalIndex);

		auto GPUs = RenderInstance->EnumerateAvailableDevices();
		auto Device = PhysicalDevice->CreateDevice();

		auto Swapchain = Device->CreateSwapchain({ 1280, 720 }, 3);

		static constexpr size_t TestBufferSize = 1024;
		auto CPUBuffer = Device->CreateBuffer(1024, Hermes::RenderInterface::ResourceUsageType::CPUAccessible | Hermes::RenderInterface::ResourceUsageType::CopySource);
		auto* Data = (Hermes::uint8*)CPUBuffer->Map();
		for (size_t Offset = 0; Offset < TestBufferSize; Offset++)
			Data[Offset] = (Hermes::uint8)Offset;
		CPUBuffer->Unmap();

		auto GPUBuffer = Device->CreateBuffer(1024, Hermes::RenderInterface::ResourceUsageType::CopyDestination | Hermes::RenderInterface::ResourceUsageType::CopySource);

		auto CommandBuffer = Device->GetQueue(Hermes::RenderInterface::QueueType::Transfer)->CreateCommandBuffer(true);
		CommandBuffer->BeginRecording();
		std::vector<Hermes::RenderInterface::BufferCopyRegion> CopyRegions;
		Hermes::RenderInterface::BufferCopyRegion CopyRegion = {};
		CopyRegion.SourceOffset = 0;
		CopyRegion.DestinationOffset = 0;
		CopyRegion.NumBytes = TestBufferSize;
		CopyRegions.push_back(CopyRegion);
		CommandBuffer->CopyBuffer(CPUBuffer, GPUBuffer, CopyRegions);
		CommandBuffer->EndRecording();
		Device->GetQueue(Hermes::RenderInterface::QueueType::Transfer)->SubmitCommandBuffer(CommandBuffer, std::nullopt);
		Device->WaitForIdle();

		auto Fence = Device->CreateFence();
		
		auto CPUBuffer2 = Device->CreateBuffer(1024, Hermes::RenderInterface::ResourceUsageType::CopyDestination | Hermes::RenderInterface::ResourceUsageType::CPUAccessible);
		CommandBuffer->BeginRecording();
		CommandBuffer->CopyBuffer(GPUBuffer, CPUBuffer2, CopyRegions);
		CommandBuffer->EndRecording();
		Device->GetQueue(Hermes::RenderInterface::QueueType::Transfer)->SubmitCommandBuffer(CommandBuffer, Fence);
		Fence->Wait(UINT64_MAX);
		auto* Data2 = (Hermes::uint8*)CPUBuffer2->Map();
		HERMES_ASSERT(Data2[0] == 0x00 && Data2[1] == 0x01 && Data2[2] == 0x02); // etc.(better check in debug mode)

		auto VertexShader = Device->CreateShader(L"Shaders/Bin/basic_vert.glsl.spv", Hermes::RenderInterface::ShaderType::VertexShader);
		auto FragmentShader = Device->CreateShader(L"Shaders/Bin/basic_frag.glsl.spv", Hermes::RenderInterface::ShaderType::FragmentShader);

		Hermes::RenderInterface::RenderPassDescription Description = {};
		Description.Attachments.push_back({});
		Description.Attachments[0].LayoutBeforeBegin = Hermes::RenderInterface::ImageLayout::Undefined;
		Description.Attachments[0].LayoutAtEnd = Hermes::RenderInterface::ImageLayout::ReadyForPresentation;
		Description.Attachments[0].Format = Swapchain->GetImageFormat();
		Description.Attachments[0].LoadOp = Hermes::RenderInterface::AttachmentLoadOp::Clear;
		Description.Attachments[0].StoreOp = Hermes::RenderInterface::AttachmentStoreOp::Store;
		Description.Attachments[0].StencilLoadOp = Hermes::RenderInterface::AttachmentLoadOp::Undefined;
		Description.Attachments[0].StencilStoreOp = Hermes::RenderInterface::AttachmentStoreOp::Undefined;

		Description.Subpasses.push_back({});
		Description.Subpasses[0].IsDepthStencilAttachmentUsed = false;
		Description.Subpasses[0].ColorAttachments.push_back({});
		Description.Subpasses[0].ColorAttachments[0].Index = 0;
		Description.Subpasses[0].ColorAttachments[0].Layout = Hermes::RenderInterface::ImageLayout::ColorAttachmentOptimal;

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
