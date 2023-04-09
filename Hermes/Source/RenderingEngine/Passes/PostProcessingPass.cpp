#include "PostProcessingPass.h"

#include "Core/Profiling.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/FrameGraph/Graph.h"
#include "RenderingEngine/FrameGraph/Resource.h"
#include "RenderingEngine/Renderer.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Device.h"
#include "Vulkan/Pipeline.h"

namespace Hermes
{
	PostProcessingPass::PostProcessingPass()
	{
		auto& Device = Renderer::GetDevice();
		auto& DescriptorAllocator = Renderer::GetDescriptorAllocator();

		VkDescriptorSetLayoutBinding InputColorBinding = {};
		InputColorBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		InputColorBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		InputColorBinding.binding = 0;
		InputColorBinding.descriptorCount = 1;

		DescriptorLayout = Device.CreateDescriptorSetLayout({ InputColorBinding });
		DescriptorSet = DescriptorAllocator.Allocate(*DescriptorLayout);

		Vulkan::SamplerDescription InputSamplerDesc = {
			.MagnificationFilter = VK_FILTER_NEAREST,
			.MinificationFilter = VK_FILTER_NEAREST,
			.MipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
			.AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.CoordinateSystem = Vulkan::CoordinateSystem::Normalized,
			.Anisotropy = false,
			.AnisotropyLevel = 0.0f,
			.MinLOD = 0.0f,
			.MaxLOD = 0.0f,
			.LODBias = 0.0f
		};

		InputColorSampler = Device.CreateSampler(InputSamplerDesc);

		Attachment InputColor = {};
		InputColor.Name = "InputColor";
		InputColor.LoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		InputColor.StencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		InputColor.Binding = BindingMode::SampledImage;

		Attachment OutputColor = {};
		OutputColor.Name = "OutputColor";
		OutputColor.LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		OutputColor.StencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		memset(&OutputColor.ClearColor, 0, sizeof(OutputColor.ClearColor));
		OutputColor.Binding = BindingMode::ColorAttachment;

		Description.Attachments = { InputColor, OutputColor };
		Description.Callback = [this](const PassCallbackInfo& CallbackInfo) { PassCallback(CallbackInfo); };
	}

	const PassDesc& PostProcessingPass::GetPassDescription() const
	{
		return Description;
	}

	void PostProcessingPass::PassCallback(const PassCallbackInfo& CallbackInfo)
	{
		HERMES_PROFILE_FUNC();

		const auto* Framebuffer = std::get<const Vulkan::ImageView*>(CallbackInfo.Resources.at("OutputColor"));
		HERMES_ASSERT(Framebuffer);

		auto FramebufferDimensions = Framebuffer->GetDimensions();
		auto ViewportDimensions = Vec2(FramebufferDimensions);

		if (!Pipeline)
			CreatePipeline(Framebuffer->GetFormat());

		auto& CommandBuffer = CallbackInfo.CommandBuffer;
		auto& Metrics = CallbackInfo.Metrics;

		HERMES_ASSERT(CallbackInfo.Resources.contains("InputColor"));
		DescriptorSet->UpdateWithImageAndSampler(0, 0, *std::get<const Vulkan::ImageView*>(CallbackInfo.Resources.at("InputColor")), *InputColorSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		CommandBuffer.BindPipeline(*Pipeline);
		Metrics.PipelineBindCount++;

		CommandBuffer.SetViewport({ 0.0f, 0.0f, ViewportDimensions.X, ViewportDimensions.Y, 0.0f, 1.0f });
		CommandBuffer.SetScissor({ { 0, 0 }, { FramebufferDimensions.X, FramebufferDimensions.Y } });

		CommandBuffer.BindDescriptorSet(*DescriptorSet, *Pipeline, 0);
		Metrics.DescriptorSetBindCount++;
		CommandBuffer.Draw(6, 1, 0, 0);
		Metrics.DrawCallCount++;
	}

	void PostProcessingPass::CreatePipeline(VkFormat ColorAttachmentFormat)
	{
		auto& ShaderCache = Renderer::GetShaderCache();

		const auto& VertexShader = ShaderCache.GetShader("/Shaders/Bin/fs_vert.glsl.spv", VK_SHADER_STAGE_VERTEX_BIT);
		const auto& FragmentShader = ShaderCache.GetShader("/Shaders/Bin/fs_postprocessing_frag.glsl.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		Vulkan::PipelineDescription Desc = {};

		Desc.ShaderStages = { &VertexShader, &FragmentShader };
		Desc.DescriptorSetLayouts = { DescriptorLayout.get() };
		Desc.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		Desc.PolygonMode = VK_POLYGON_MODE_FILL;
		Desc.CullMode = VK_CULL_MODE_BACK_BIT;
		Desc.FaceDirection = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		Desc.IsDepthTestEnabled = false;
		Desc.IsDepthWriteEnabled = false;
		Desc.DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		Pipeline = Renderer::GetDevice().CreatePipeline(Desc, { &ColorAttachmentFormat, 1}, std::nullopt);
	}
}
