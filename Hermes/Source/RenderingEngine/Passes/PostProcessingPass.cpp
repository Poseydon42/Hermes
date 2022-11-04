#include "PostProcessingPass.h"

#include "Core/Profiling.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "Vulkan/Device.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Swapchain.h"

namespace Hermes
{
	PostProcessingPass::PostProcessingPass()
	{
		auto& Device = Renderer::Get().GetActiveDevice();
		auto& DescriptorAllocator = Renderer::Get().GetDescriptorAllocator();

		VkDescriptorSetLayoutBinding InputColorBinding = {};
		InputColorBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		InputColorBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		InputColorBinding.binding = 0;
		InputColorBinding.descriptorCount = 1;

		DescriptorLayout = Device.CreateDescriptorSetLayout({ InputColorBinding });
		DescriptorSet = DescriptorAllocator.Allocate(*DescriptorLayout);

		VertexShader = Device.CreateShader(L"Shaders/Bin/fs_vert.glsl.spv", VK_SHADER_STAGE_VERTEX_BIT);
		FragmentShader = Device.CreateShader(L"Shaders/Bin/fs_postprocessing_frag.glsl.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		Attachment InputColor = {};
		InputColor.Name = L"InputColor";
		InputColor.LoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		InputColor.StencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		InputColor.Binding = BindingMode::InputAttachment;

		Attachment OutputColor = {};
		OutputColor.Name = L"OutputColor";
		OutputColor.LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		OutputColor.StencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		memset(&OutputColor.ClearColor, 0, sizeof(OutputColor.ClearColor));
		OutputColor.Binding = BindingMode::ColorAttachment;

		Description.Attachments = { InputColor, OutputColor };
		Description.Callback.Bind<PostProcessingPass, &PostProcessingPass::PassCallback>(this);
	}

	const PassDesc& PostProcessingPass::GetPassDescription() const
	{
		return Description;
	}

	void PostProcessingPass::PassCallback(Vulkan::CommandBuffer& CommandBuffer,
	                                      const Vulkan::RenderPass& PassInstance,
	                                      const std::vector<std::pair<const Vulkan::Image*, const Vulkan::ImageView*>>&
	                                      Attachments, const Scene&, const GeometryList&, FrameMetrics& Metrics,
	                                      bool ResourcesWereRecreated)
	{
		HERMES_PROFILE_FUNC();
		if (ResourcesWereRecreated || !IsPipelineCreated)
		{
			RecreatePipeline(PassInstance);
			IsPipelineCreated = true;
		}

		HERMES_ASSERT(Attachments.size() == 2 && Attachments[0].second != nullptr);
		DescriptorSet->UpdateWithImage(0, 0, *Attachments[0].second, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		CommandBuffer.BindPipeline(*Pipeline);
		Metrics.PipelineBindCount++;
		CommandBuffer.BindDescriptorSet(*DescriptorSet, *Pipeline, 0);
		Metrics.DescriptorSetBindCount++;
		CommandBuffer.Draw(6, 1, 0, 0);
		Metrics.DrawCallCount++;
	}

	void PostProcessingPass::RecreatePipeline(const Vulkan::RenderPass& Pass)
	{
		Vulkan::PipelineDescription Desc = {};

		Desc.ShaderStages = { VertexShader.get(), FragmentShader.get() };
		Desc.DescriptorSetLayouts = { DescriptorLayout.get() };
		Desc.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		Desc.Viewport.x = 0;
		Desc.Viewport.y = 0;
		Desc.Viewport.width = static_cast<float>(Renderer::Get().GetSwapchain().GetDimensions().X);
		Desc.Viewport.height = static_cast<float>(Renderer::Get().GetSwapchain().GetDimensions().Y);
		Desc.Scissor.offset = { 0, 0 };
		Desc.Scissor.extent = {
			Renderer::Get().GetSwapchain().GetDimensions().X, Renderer::Get().GetSwapchain().GetDimensions().Y
		};
		Desc.PolygonMode = VK_POLYGON_MODE_FILL;
		Desc.CullMode = VK_CULL_MODE_BACK_BIT;
		Desc.FaceDirection = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		Desc.IsDepthTestEnabled = false;
		Desc.IsDepthWriteEnabled = false;

		Pipeline = Renderer::Get().GetActiveDevice().CreatePipeline(Pass, Desc);
	}
}
