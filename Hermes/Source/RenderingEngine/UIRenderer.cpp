#include "UIRenderer.h"

#include "Core/Profiling.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/SharedData.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Device.h"
#include "Vulkan/Fence.h"
#include "Vulkan/Queue.h"

namespace Hermes
{
	UIRenderer::UIRenderer()
	{
		auto& Device = Renderer::GetDevice();

		VkDescriptorSetLayoutBinding RectangleListBinding = {
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr
		};
		auto DescriptorSetLayout = Device.CreateDescriptorSetLayout({ RectangleListBinding });
		DescriptorSet = Renderer::GetDescriptorAllocator().Allocate(*DescriptorSetLayout);

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
		Pipeline = Device.CreatePipeline(Desc, { &DestinationImageFormat, 1 }, std::nullopt);

		RectangleListBuffer = Device.CreateBuffer(sizeof(RectanglePrimitive) * MaxRectangleCount, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true);
		DescriptorSet->UpdateWithBuffer(0, 0, *RectangleListBuffer, 0, static_cast<uint32>(RectangleListBuffer->GetSize()));
	}

	Rect2Dui UIRenderer::PrepareToRender(const UI::Widget& RootWidget, Vec2ui RequiredDimensions)
	{
		HERMES_PROFILE_FUNC();

		if (RequiredDimensions != CurrentDimensions)
		{
			HERMES_LOG_INFO("UI renderer: resided to %u x %u", RequiredDimensions.X, RequiredDimensions.Y);
			CurrentDimensions = RequiredDimensions;
			RecreateDestinationImage();
		}

		DrawingContext = {};
		Rect2D AvailableRect = { { 0, 0 }, Vec2(CurrentDimensions) };
		RootWidget.Draw(DrawingContext, AvailableRect);

		HERMES_ASSERT(DrawingContext.GetRectangles().size() < MaxRectangleCount);
		auto* NextRectangle = static_cast<RectanglePrimitive*>(RectangleListBuffer->Map());
		for (const auto& SourceRectangle : DrawingContext.GetRectangles())
		{
			NextRectangle->Min = Vec2(SourceRectangle.Rect.Min) / Vec2(CurrentDimensions);
			NextRectangle->Max = Vec2(SourceRectangle.Rect.Max) / Vec2(CurrentDimensions);
			NextRectangle->Color = Vec4(SourceRectangle.Color, 1.0f);
			NextRectangle++;
		}
		RectangleListBuffer->Unmap();

		PushConstants = {
			.RectangleCount = static_cast<uint32>(DrawingContext.GetRectangles().size())
		};

		return DrawingContext.GetViewport();
	}

	std::pair<const Vulkan::Image*, VkImageLayout> UIRenderer::Render(const Vulkan::Image& RenderedScene, VkImageLayout RenderedSceneLayout)
	{
		HERMES_PROFILE_FUNC();


		/*
		 * Allocating command buffer
		 */
		auto& GraphicsQueue = Renderer::GetDevice().GetQueue(VK_QUEUE_GRAPHICS_BIT);
		auto CommandBuffer = GraphicsQueue.CreateCommandBuffer();
		CommandBuffer->BeginRecording();


		/*
		 * Copying the rendered scene into the destination image
		 */
		VkImageMemoryBarrier BeforeBlitBarriers[2];
		// Destination image to TRANSFER_DST_OPTIMAL
		BeforeBlitBarriers[0] = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = DestinationImage->GetImage(),
			.subresourceRange = DestinationImage->GetFullSubresourceRange()
		};
		// Scene image to TRANSFER_SRC_OPTIMAL
		BeforeBlitBarriers[1] = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
			.oldLayout = RenderedSceneLayout,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = RenderedScene.GetImage(),
			.subresourceRange = RenderedScene.GetFullSubresourceRange()
		};
		CommandBuffer->InsertImageMemoryBarriers(BeforeBlitBarriers, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		auto SceneViewport = DrawingContext.GetViewport();
		VkImageBlit Blit = {
			.srcSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 },
			.srcOffsets = { { 0, 0, 0 }, { static_cast<int32>(RenderedScene.GetDimensions().X), static_cast<int32>(RenderedScene.GetDimensions().Y), 1 } },
			.dstSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 },
			.dstOffsets = { { static_cast<int32>(SceneViewport.Min.X), static_cast<int32>(SceneViewport.Min.Y), 0 }, { static_cast<int32>(SceneViewport.Max.X), static_cast<int32>(SceneViewport.Max.Y), 1 } }
		};
		CommandBuffer->BlitImage(RenderedScene, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, *DestinationImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { &Blit, 1 }, VK_FILTER_NEAREST);

		VkImageMemoryBarrier AfterBlitBarriers[2];
		// Scene image back to its original layout
		AfterBlitBarriers[0] = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
			.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.newLayout = RenderedSceneLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = RenderedScene.GetImage(),
			.subresourceRange = RenderedScene.GetFullSubresourceRange()
		};
		// Destination image to COLOR_ATTACHMENT_OPTIMAL
		AfterBlitBarriers[1] = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = DestinationImage->GetImage(),
			.subresourceRange = DestinationImage->GetFullSubresourceRange()
		};
		CommandBuffer->InsertImageMemoryBarriers(AfterBlitBarriers, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);


		/*
		 * Rendering UI
		 */
		VkRect2D RenderingArea = {
			.offset = { 0, 0 },
			.extent = { CurrentDimensions.X, CurrentDimensions.Y }
		};
		VkRenderingAttachmentInfo Attachment = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.pNext = nullptr,
			.imageView = DestinationImageView->GetImageView(),
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.resolveMode = VK_RESOLVE_MODE_NONE,
			.resolveImageView = VK_NULL_HANDLE,
			.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = {}
		};
		CommandBuffer->BeginRendering(RenderingArea, { &Attachment, 1 }, std::nullopt, std::nullopt);

		CommandBuffer->BindPipeline(*Pipeline);
		VkViewport Viewport = {
			.x = 0.0f,
			.y = 0.0f,
			.width = static_cast<float>(CurrentDimensions.X),
			.height = static_cast<float>(CurrentDimensions.Y),
			.minDepth = 0.0f,
			.maxDepth = 0.0f
		};
		CommandBuffer->SetViewport(Viewport);
		VkRect2D Scissor = {
			.offset = { 0, 0 },
			.extent = { CurrentDimensions.X, CurrentDimensions.Y }
		};
		CommandBuffer->SetScissor(Scissor);

		CommandBuffer->BindDescriptorSet(*DescriptorSet, *Pipeline, 0);
		CommandBuffer->UploadPushConstants(*Pipeline, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, &PushConstants, sizeof(PushConstants), 0);

		CommandBuffer->Draw(6, 1, 0, 0);

		CommandBuffer->EndRendering();
		CommandBuffer->EndRecording();


		/*
		 * Submitting the command buffer and waiting for its fence
		 */
		auto Fence = Renderer::GetDevice().CreateFence();
		GraphicsQueue.SubmitCommandBuffer(*CommandBuffer, Fence.get());
		Fence->Wait(UINT64_MAX);

		return std::make_pair(DestinationImage.get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}

	void UIRenderer::RecreateDestinationImage()
	{
		DestinationImage = Renderer::GetDevice().CreateImage(CurrentDimensions, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, DestinationImageFormat, 1);
		DestinationImageView = DestinationImage->CreateDefaultImageView();
	}
}
