#include "UIPass.h"

#include "Core/Profiling.h"
#include "RenderingEngine/FrameGraph/Graph.h"
#include "RenderingEngine/FrameGraph/Resource.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/SharedData.h"
#include "Vulkan/CommandBuffer.h"

namespace Hermes
{
	UIPass::UIPass()
	{
		Description.Attachments.emplace_back("Framebuffer", VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VkClearValue{}, BindingMode::ColorAttachment);
		Description.Type = PassType::Graphics;
		Description.Callback = [this](const PassCallbackInfo& CallbackInfo) { PassCallback(CallbackInfo); };
		
		auto& Device = Renderer::GetDevice();
		auto& DescriptorAllocator = Renderer::GetDescriptorAllocator();


		DescriptorSetLayout = Device.CreateDescriptorSetLayout({
			{ .binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr },
		});
		DescriptorSet = DescriptorAllocator.Allocate(*DescriptorSetLayout);
	}

	const PassDesc& UIPass::GetPassDescription() const
	{
		return Description;
	}

	void UIPass::SetDrawingContext(const UI::DrawingContext* NewDrawingContext)
	{
		DrawingContext = NewDrawingContext;
	}

	void UIPass::PassCallback(const PassCallbackInfo& CallbackInfo)
	{
		HERMES_PROFILE_FUNC();

		const auto* Framebuffer = std::get<const Vulkan::ImageView*>(CallbackInfo.Resources.at("Framebuffer"));
		HERMES_ASSERT(Framebuffer);
		auto FramebufferDimensions = Framebuffer->GetDimensions();
		auto ViewportDimensions = Vec2(FramebufferDimensions);

		if (!Pipeline)
			CreatePipeline(Framebuffer->GetFormat());

		auto& CommandBuffer = CallbackInfo.CommandBuffer;
		auto& Metrics = CallbackInfo.Metrics;

		std::vector<RectanglePrimitive> Rectangles;

		for (const auto& Rectangle : DrawingContext->GetRectangles())
		{
			Rectangles.emplace_back(Vec2(Rectangle.Rect.Min) / ViewportDimensions, Vec2(Rectangle.Rect.Max) / ViewportDimensions, Vec4(Rectangle.Color, 1.0f));
		}

		auto RectangleListSize = Rectangles.size() * sizeof(Rectangles[0]);
		if (!RectangleListBuffer || RectangleListBuffer->GetSize() < RectangleListSize)
		{
			// NOTE: easier to create a small buffer even if it's unused rather than handle 2 cases
			auto RectangleListBufferSize = Math::Max(RectangleListSize, sizeof(RectanglePrimitive));
			RectangleListBuffer = Renderer::GetDevice().CreateBuffer(RectangleListBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true);
			DescriptorSet->UpdateWithBuffer(0, 0, *RectangleListBuffer, 0, static_cast<uint32>(RectangleListBufferSize));
		}

		auto* RectangleListBufferMemory = RectangleListBuffer->Map();
		memcpy(RectangleListBufferMemory, Rectangles.data(), RectangleListSize);
		RectangleListBuffer->Unmap();

		CommandBuffer.BindPipeline(*Pipeline);
		Metrics.PipelineBindCount++;

		CommandBuffer.SetViewport({ 0.0f, 0.0f, ViewportDimensions.X, ViewportDimensions.Y, 0.0f, 1.0f });
		CommandBuffer.SetScissor({ { 0, 0 }, { FramebufferDimensions.X, FramebufferDimensions.Y } });

		CommandBuffer.BindDescriptorSet(*DescriptorSet, *Pipeline, 0);
		Metrics.DescriptorSetBindCount++;
		UIShaderPushConstants PushConstants = {};
		PushConstants.RectangleCount = static_cast<uint32>(Rectangles.size());
		CommandBuffer.UploadPushConstants(*Pipeline, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, &PushConstants, sizeof(PushConstants), 0);
		CommandBuffer.Draw(6, 1, 0, 0);
		Metrics.DrawCallCount++;
	}

	void UIPass::CreatePipeline(VkFormat ColorAttachmentFormat)
	{
		auto& Device = Renderer::GetDevice();
		auto& ShaderCache = Renderer::GetShaderCache();

		const auto& VertexShader = ShaderCache.GetShader("/Shaders/Bin/fs_ui_vert.glsl.spv", VK_SHADER_STAGE_VERTEX_BIT);
		const auto& FragmentShader = ShaderCache.GetShader("/Shaders/Bin/fs_ui_frag.glsl.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		Vulkan::PipelineDescription Desc = {
			.PushConstants = { VkPushConstantRange {.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, .offset = 0, .size = sizeof(UIShaderPushConstants)}},
			.DescriptorSetLayouts = { DescriptorSetLayout.get() },
			.ShaderStages = { &VertexShader, &FragmentShader },
			.VertexInputBindings = {},
			.VertexInputAttributes = {},
			.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.PolygonMode = VK_POLYGON_MODE_FILL,
			.CullMode = VK_CULL_MODE_NONE,
			.FaceDirection = VK_FRONT_FACE_CLOCKWISE,
			.AttachmentColorBlending = {
				VkPipelineColorBlendAttachmentState {
					.blendEnable = true,
					.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
					.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
					.colorBlendOp = VK_BLEND_OP_ADD,
					.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
					.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
					.alphaBlendOp = VK_BLEND_OP_ADD,
					.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
				}
			},
			.IsDepthTestEnabled = false,
			.IsDepthWriteEnabled = false,
			.DepthCompareOperator = VK_COMPARE_OP_NEVER,
			.DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR }
		};

		Pipeline = Device.CreatePipeline(Desc, { &ColorAttachmentFormat, 1 }, std::nullopt);
	}
}
