#include "UIPass.h"

#include "Core/Profiling.h"
#include "RenderingEngine/Renderer.h"
#include "Vulkan/Buffer.h"

namespace Hermes
{
	UIPass::UIPass()
	{
		Description.Attachments.emplace_back("Framebuffer", VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VkClearValue{}, BindingMode::ColorAttachment);
		Description.Type = PassType::Graphics;
		Description.Callback = [this](const PassCallbackInfo& CallbackInfo) { PassCallback(CallbackInfo); };

		auto& Renderer = Renderer::Get();
		auto& Device = Renderer.GetActiveDevice();
		auto& DescriptorAllocator = Renderer.GetDescriptorAllocator();


		DescriptorSetLayout = Device.CreateDescriptorSetLayout({
			{ .binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr },
		});
		DescriptorSet = DescriptorAllocator.Allocate(*DescriptorSetLayout);
	}

	const PassDesc& UIPass::GetPassDescription() const
	{
		return Description;
	}

	void UIPass::SetRootWidget(const UI::Widget* NewRootWidget)
	{
		RootWidget = NewRootWidget;
	}

	void UIPass::PassCallback(const PassCallbackInfo& CallbackInfo)
	{
		HERMES_PROFILE_FUNC();

		auto FramebufferDimensions = std::get<const Vulkan::ImageView*>(CallbackInfo.Resources.at("Framebuffer"))->GetDimensions();
		auto ViewportDimensions = Vec2(FramebufferDimensions);

		if (!Pipeline)
			CreatePipeline(*CallbackInfo.RenderPass);

		auto& Renderer = Renderer::Get();
		auto& CommandBuffer = CallbackInfo.CommandBuffer;
		auto& Metrics = CallbackInfo.Metrics;

		std::vector<RectanglePrimitive> Rectangles;

		Rect2D RootWidgetRect = {
			.Min = { 0.0f, 0.0f },
			.Max = { ViewportDimensions.X, ViewportDimensions.Y }
		};
		Vec2 RootWidgetDimensions = { RootWidgetRect.Width(), RootWidgetRect.Height() };

		UI::DrawingContext DrawingContext;
		RootWidget->Draw(DrawingContext, RootWidgetRect);

		for (const auto& Rectangle : DrawingContext.GetRectangles())
		{
			Rectangles.emplace_back(Vec2(Rectangle.Rect.Min) / RootWidgetDimensions, Vec2(Rectangle.Rect.Max) / RootWidgetDimensions, Vec4(Rectangle.Color, 1.0f));
		}

		auto RectangleListSize = Rectangles.size() * sizeof(Rectangles[0]);
		if (!RectangleListBuffer || RectangleListBuffer->GetSize() < RectangleListSize)
		{
			// NOTE: easier to create a small buffer even if it's unused rather than handle 2 cases
			auto RectangleListBufferSize = Math::Max(RectangleListSize, sizeof(RectanglePrimitive));
			RectangleListBuffer = Renderer.GetActiveDevice().CreateBuffer(RectangleListBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true);
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

	void UIPass::CreatePipeline(const Vulkan::RenderPass& RenderPass)
	{
		auto& Renderer = Renderer::Get();
		auto& Device = Renderer.GetActiveDevice();
		auto& ShaderCache = Renderer.GetShaderCache();

		const auto& VertexShader = ShaderCache.GetShader("/Shaders/Bin/fs_ui_vert.glsl.spv", VK_SHADER_STAGE_VERTEX_BIT);
		const auto& FragmentShader = ShaderCache.GetShader("/Shaders/Bin/fs_ui_frag.glsl.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		Vulkan::PipelineDescription Desc = {
			.PushConstants = { VkPushConstantRange { .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, .offset = 0, .size = sizeof(UIShaderPushConstants)}},
			.DescriptorSetLayouts = { DescriptorSetLayout.get() },
			.ShaderStages = { &VertexShader, &FragmentShader },
			.VertexInputBindings = {},
			.VertexInputAttributes = {},
			.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.PolygonMode = VK_POLYGON_MODE_FILL,
			.CullMode = VK_CULL_MODE_NONE,
			.FaceDirection = VK_FRONT_FACE_CLOCKWISE,
			.IsDepthTestEnabled = false,
			.IsDepthWriteEnabled = false,
			.DepthCompareOperator = VK_COMPARE_OP_NEVER,
			.DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR }
		};

		Pipeline = Device.CreatePipeline(RenderPass, Desc);
	}
}
