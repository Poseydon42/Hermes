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

	void UIPass::AddWindow(const UI::Window* Window, Vec2ui ScreenLocation)
	{
		Windows.emplace_back(Window, ScreenLocation);
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
		std::vector<std::pair<uint32, uint32>> RectangleListRange;

		for (auto [Window, Location] : Windows)
		{
			auto FirstRectangle = static_cast<uint32>(Rectangles.size());
			CollectPrimitives(Window, Rectangles);
			auto RectangleCount = static_cast<uint32>(Rectangles.size()) - FirstRectangle;
			RectangleListRange.emplace_back(FirstRectangle, RectangleCount);
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
		for (size_t WindowIndex = 0; WindowIndex < Windows.size(); WindowIndex++)
		{
			UIShaderPushConstants PushConstants = {};
			PushConstants.TopLeft = Vec2(Windows[WindowIndex].second) / ViewportDimensions;
			PushConstants.BottomRight = Vec2(Windows[WindowIndex].second + Windows[WindowIndex].first->GetDimensions()) / Vec2(ViewportDimensions);
			PushConstants.FirstRectangle = RectangleListRange[WindowIndex].first;
			PushConstants.RectangleCount = RectangleListRange[WindowIndex].second;
			CommandBuffer.UploadPushConstants(*Pipeline, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, &PushConstants, sizeof(PushConstants), 0);
			CommandBuffer.Draw(6, 1, 0, 0);
			Metrics.DrawCallCount++;
		}

		Windows.clear();
	}

	void UIPass::CollectPrimitives(const UI::Window* Window, std::vector<RectanglePrimitive>& Rectangles)
	{
		auto Context = Window->Draw();
		auto WindowDimensions = Vec2(Window->GetDimensions());

		for (const auto& Rectangle : Context.GetRectangles())
		{
			// NOTE: rectangles in the DrawingContext object have their coordinates and dimensions set in pixels,
			//       so we need to transform them into [0;1] range relative to the window dimensions here
			Rectangles.emplace_back(Vec2(Rectangle.Rect.Min) / WindowDimensions, Vec2(Rectangle.Rect.Max) / WindowDimensions, Vec4(Rectangle.Color, 1.0f));
		}
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
