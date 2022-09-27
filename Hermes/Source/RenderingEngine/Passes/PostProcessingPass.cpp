#include "PostProcessingPass.h"

#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/Shader.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"

namespace Hermes
{
	PostProcessingPass::PostProcessingPass()
	{
		auto& Device = Renderer::Get().GetActiveDevice();
		auto& DescriptorAllocator = Renderer::Get().GetDescriptorAllocator();

		RenderInterface::DescriptorBinding InputColorBinding = {};
		InputColorBinding.Type = RenderInterface::DescriptorType::InputAttachment;
		InputColorBinding.Shader = RenderInterface::ShaderType::FragmentShader;
		InputColorBinding.Index = 0;
		InputColorBinding.DescriptorCount = 1;

		DescriptorLayout = Device.CreateDescriptorSetLayout({ InputColorBinding });
		DescriptorSet = DescriptorAllocator.Allocate(*DescriptorLayout);

		VertexShader = Device.CreateShader(L"Shaders/Bin/fs_vert.glsl.spv", RenderInterface::ShaderType::VertexShader);
		FragmentShader = Device.CreateShader(L"Shaders/Bin/fs_postprocessing_frag.glsl.spv", RenderInterface::ShaderType::FragmentShader);

		Drain InputColorDrain = {};
		InputColorDrain.Name = L"InputColor";
		InputColorDrain.LoadOp = RenderInterface::AttachmentLoadOp::Load;
		InputColorDrain.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;
		InputColorDrain.Binding = BindingMode::InputAttachment;

		Drain OutputColorDrain = {};
		OutputColorDrain.Name = L"OutputColor";
		OutputColorDrain.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		OutputColorDrain.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;
		memset(&OutputColorDrain.ClearColor, 0, sizeof(OutputColorDrain.ClearColor));
		OutputColorDrain.Binding = BindingMode::ColorAttachment;

		Description.Drains = { InputColorDrain, OutputColorDrain };
		Description.Callback.Bind<PostProcessingPass, &PostProcessingPass::PassCallback>(this);
	}

	const PassDesc& PostProcessingPass::GetPassDescription() const
	{
		return Description;
	}

	void PostProcessingPass::PassCallback(RenderInterface::CommandBuffer& CommandBuffer,
	                                      const RenderInterface::RenderPass& PassInstance,
	                                      const std::vector<std::pair<
		                                      const RenderInterface::Image*, const RenderInterface::ImageView*>>& Drains,
	                                      const Scene&, bool ResourcesWereRecreated)
	{
		if (ResourcesWereRecreated || !IsPipelineCreated)
		{
			RecreatePipeline(PassInstance);
			IsPipelineCreated = true;
		}

		HERMES_ASSERT(Drains.size() == 2 && Drains[0].second != nullptr);
		DescriptorSet->UpdateWithImage(0, 0, *Drains[0].second, RenderInterface::ImageLayout::ShaderReadOnlyOptimal);

		CommandBuffer.BindPipeline(*Pipeline);
		CommandBuffer.BindDescriptorSet(*DescriptorSet, *Pipeline, 0);
		CommandBuffer.Draw(6, 1, 0, 0);
	}

	void PostProcessingPass::RecreatePipeline(const RenderInterface::RenderPass& Pass)
	{
		RenderInterface::PipelineDescription Desc = {};

		Desc.ShaderStages = { VertexShader.get(), FragmentShader.get() };
		Desc.DescriptorLayouts = { DescriptorLayout.get() };
		Desc.InputAssembler.Topology = RenderInterface::TopologyType::TriangleFan;
		Desc.Viewport.Origin = { 0, 0 };
		Desc.Viewport.Dimensions = Renderer::Get().GetSwapchain().GetSize();
		Desc.Rasterizer.Fill = RenderInterface::FillMode::Fill;
		Desc.Rasterizer.Cull = RenderInterface::CullMode::Back;
		Desc.Rasterizer.Direction = RenderInterface::FaceDirection::CounterClockwise;
		Desc.DepthStencilStage.IsDepthTestEnabled = true;
		Desc.DepthStencilStage.ComparisonMode = RenderInterface::ComparisonOperator::AlwaysSucceed;
		Desc.DepthStencilStage.IsDepthWriteEnabled = false;

		Pipeline = Renderer::Get().GetActiveDevice().CreatePipeline(Pass, Desc);
	}
}
