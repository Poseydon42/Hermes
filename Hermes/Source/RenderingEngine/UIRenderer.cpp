#include "UIRenderer.h"

#include "Core/Misc/UTF8StringView.h"
#include "Core/Profiling.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/FontPack.h"
#include "RenderingEngine/GPUInteractionUtilities.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/SharedData.h"
#include "UIEngine/TextLayout.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Device.h"
#include "Vulkan/Fence.h"
#include "Vulkan/Queue.h"

namespace Hermes
{
	struct TextVertex
	{
		Vec2 Position;
		Vec2 TextureCoordinates;
	};

	UIRenderer::UIRenderer()
	{
		auto& Device = Renderer::GetDevice();
		auto& ShaderCache = Renderer::GetShaderCache();


		/*
		 * Rectangle pipeline initialization
		 */
		VkDescriptorSetLayoutBinding RectangleListBinding = {
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr
		};
		auto RectangleDescriptorSetLayout = Device.CreateDescriptorSetLayout({ RectangleListBinding });
		RectangleDescriptorSet = Renderer::GetDescriptorAllocator().Allocate(*RectangleDescriptorSetLayout);

		VkVertexInputBindingDescription RectangleMeshBufferVertexInputBinding = {
			.binding = 0,
			.stride = sizeof(Vec2),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};
		VkVertexInputAttributeDescription RectangleMeshBufferVertexPositionInputAttribute = {
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = 0
		};

		Vulkan::PipelineDescription RectanglePipelineDesc = {
			.DescriptorSetLayouts = { RectangleDescriptorSetLayout.get() },
			.ShaderStages = {
				&ShaderCache.GetShader("/Shaders/Bin/fs_ui_vert.glsl.spv", VK_SHADER_STAGE_VERTEX_BIT),
				&ShaderCache.GetShader("/Shaders/Bin/fs_ui_frag.glsl.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
			},
			.VertexInputBindings = { RectangleMeshBufferVertexInputBinding },
			.VertexInputAttributes = { RectangleMeshBufferVertexPositionInputAttribute },
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
		RectanglePipeline = Device.CreatePipeline(RectanglePipelineDesc, { &DestinationImageFormat, 1 }, std::nullopt);
		

		/*
		 * Text pipeline initialization
		 */
		VkDescriptorSetLayoutBinding GlyphTextureBinding = {
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr
		};
		auto TextDescriptorSetLayout = Device.CreateDescriptorSetLayout({ GlyphTextureBinding });
		TextDescriptorSet = Renderer::GetDescriptorAllocator().Allocate(*TextDescriptorSetLayout);

		VkVertexInputBindingDescription TextVertexInputBindingDescription = {
			.binding = 0,
			.stride = sizeof(TextVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};
		VkVertexInputAttributeDescription TextVertexPositionAttribute = {
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(TextVertex, Position)
		};
		VkVertexInputAttributeDescription TextTextureCoordinateAttribute = {
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(TextVertex, TextureCoordinates)
		};
		Vulkan::PipelineDescription TextPipelineDesc = {
			.PushConstants = {},
			.DescriptorSetLayouts = { TextDescriptorSetLayout.get() },
			.ShaderStages = {
				&ShaderCache.GetShader("/Shaders/Bin/fs_ui_text_vert.glsl.spv", VK_SHADER_STAGE_VERTEX_BIT),
				&ShaderCache.GetShader("/Shaders/Bin/fs_ui_text_frag.glsl.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
			},
			.VertexInputBindings = { TextVertexInputBindingDescription },
			.VertexInputAttributes = { TextVertexPositionAttribute, TextTextureCoordinateAttribute },
			.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.Viewport = {},
			.Scissor = {},
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
			.BlendingConstants = {},
			.IsDepthTestEnabled = false,
			.IsDepthWriteEnabled = false,
			.DepthCompareOperator = VK_COMPARE_OP_ALWAYS,
			.DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR }
		};
		TextPipeline = Device.CreatePipeline(TextPipelineDesc, { &DestinationImageFormat, 1 }, std::nullopt);

		Vulkan::SamplerDescription TextFontSamplerDesc = {
			.MagnificationFilter = VK_FILTER_LINEAR,
			.MinificationFilter = VK_FILTER_LINEAR,
			.MipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.CoordinateSystem = Vulkan::CoordinateSystem::Normalized,
			.Anisotropy = false,
			.AnisotropyLevel = 0.0f,
			.MinLOD = 0.0f,
			.MaxLOD = 0.0f,
			.LODBias = 0.0f
		};
		TextFontSampler = Device.CreateSampler(TextFontSamplerDesc);
	}

	Rect2Dui UIRenderer::PrepareToRender(UI::Widget& RootWidget, Vec2ui RequiredDimensions)
	{
		HERMES_PROFILE_FUNC();

		if (RequiredDimensions != CurrentDimensions)
		{
			HERMES_LOG_INFO("UI renderer: resided to %u x %u", RequiredDimensions.X, RequiredDimensions.Y);
			CurrentDimensions = RequiredDimensions;
			RecreateDestinationImage();
		}

		{
			HERMES_PROFILE_SCOPE("UI layout");

			Rect2D RootWidgetBoundingBox = { { 0, 0 }, Vec2(CurrentDimensions) };
			RootWidget.SetBoundingBox(RootWidgetBoundingBox);

			RootWidget.Layout();
		}

		{
			HERMES_PROFILE_SCOPE("UI draw");

			UI::DrawingContext DrawingContext = {};
			RootWidget.Draw(DrawingContext);

			SceneViewport = DrawingContext.GetViewport();

			PrepareToRenderRectangles(DrawingContext);
			PrepareToRenderText(DrawingContext);
		}

		return SceneViewport;
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
		 * Beginning rendering
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
		VkViewport Viewport = {
			.x = 0.0f,
			.y = 0.0f,
			.width = static_cast<float>(CurrentDimensions.X),
			.height = static_cast<float>(CurrentDimensions.Y),
			.minDepth = 0.0f,
			.maxDepth = 0.0f
		};
		VkRect2D Scissor = {
			.offset = { 0, 0 },
			.extent = { CurrentDimensions.X, CurrentDimensions.Y }
		};
		CommandBuffer->BeginRendering(RenderingArea, { &Attachment, 1 }, std::nullopt, std::nullopt);


		/*
		 * Rendering rectangles
		 */
		if (HasRectanglesToDraw)
		{
			CommandBuffer->BindPipeline(*RectanglePipeline);
			CommandBuffer->SetViewport(Viewport);
			CommandBuffer->SetScissor(Scissor);

			CommandBuffer->BindDescriptorSet(*RectangleDescriptorSet, *RectanglePipeline, 0);
			CommandBuffer->BindVertexBuffer(*RectangleMeshBuffer);

			CommandBuffer->Draw(static_cast<uint32>(RectangleMeshBuffer->GetSize() / sizeof(Vec2)), 1, 0, 0);
		}


		/*
		 * Rendering text
		 */
		if (HasTextToDraw)
		{
			CommandBuffer->BindPipeline(*TextPipeline);
			CommandBuffer->SetViewport(Viewport);
			CommandBuffer->SetScissor(Scissor);

			CommandBuffer->BindDescriptorSet(*TextDescriptorSet, *TextPipeline, 0);
			CommandBuffer->BindVertexBuffer(*TextMeshBuffer);

			CommandBuffer->Draw(static_cast<uint32>(TextMeshBuffer->GetSize() / sizeof(TextVertex)), 1, 0, 0);
		}

		/*
		 * Finishing command buffer recording
		 */
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

	void UIRenderer::PrepareToRenderRectangles(const UI::DrawingContext& DrawingContext)
	{
		HERMES_PROFILE_FUNC();

		auto RectangleCount = DrawingContext.GetRectangles().size();
		auto VertexCount = RectangleCount * 6;
		std::vector<Vec2> RectangleVertices(VertexCount);

		std::vector<RectanglePrimitive> RectanglePrimitives(RectangleCount);

		size_t RectangleIndex = 0;
		for (const auto& Rectangle : DrawingContext.GetRectangles())
		{
			auto TopLeft = Vec2(Rectangle.Rect.Min) / Vec2(CurrentDimensions);
			auto BottomRight = Vec2(Rectangle.Rect.Max) / Vec2(CurrentDimensions);

			TopLeft = TopLeft * 2.0f - 1.0f;
			BottomRight = BottomRight * 2.0f - 1.0;

			auto TopRight = Vec2(BottomRight.X, TopLeft.Y);
			auto BottomLeft = Vec2(TopLeft.X, BottomRight.Y);

			RectangleVertices[RectangleIndex * 6 + 0] = BottomLeft;
			RectangleVertices[RectangleIndex * 6 + 1] = TopLeft;
			RectangleVertices[RectangleIndex * 6 + 2] = TopRight;
			RectangleVertices[RectangleIndex * 6 + 3] = TopRight;
			RectangleVertices[RectangleIndex * 6 + 4] = BottomRight;
			RectangleVertices[RectangleIndex * 6 + 5] = BottomLeft;
			RectanglePrimitives[RectangleIndex].Color = Rectangle.Color;


			RectangleIndex++;
		}

		// TODO: only recreate the buffer if it has to be resized
		RectangleMeshBuffer = Renderer::GetDevice().CreateBuffer(RectangleVertices.size() * sizeof(RectangleVertices[0]), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		GPUInteractionUtilities::UploadDataToGPUBuffer(RectangleVertices.data(), RectangleMeshBuffer->GetSize(), 0, *RectangleMeshBuffer);

		RectanglePrimitiveBuffer = Renderer::GetDevice().CreateBuffer(RectanglePrimitives.size() * sizeof(RectanglePrimitives[0]), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		GPUInteractionUtilities::UploadDataToGPUBuffer(RectanglePrimitives.data(), RectanglePrimitiveBuffer->GetSize(), 0, *RectanglePrimitiveBuffer);
		RectangleDescriptorSet->UpdateWithBuffer(0, 0, *RectanglePrimitiveBuffer, 0, static_cast<uint32>(RectanglePrimitiveBuffer->GetSize()));
	}

	void UIRenderer::PrepareToRenderText(const UI::DrawingContext& DrawingContext)
	{
		HERMES_PROFILE_FUNC();

		if (DrawingContext.GetDrawableTexts().empty())
		{
			HasTextToDraw = false;
			return;
		}

		for (const auto& Text : DrawingContext.GetDrawableTexts())
		{
			for (auto Char : UTF8StringView(Text.Text))
			{
				auto GlyphIndex = Text.Font->GetGlyphIndex(Char);
				HERMES_ASSERT(GlyphIndex.has_value());
				FontPack.RequestGlyph(*Text.Font, GlyphIndex.value(), Text.FontSize);
			}
		}
		FontPack.Repack();

		std::vector<TextVertex> TextVertices;
		for (const auto& Text : DrawingContext.GetDrawableTexts())
		{
			HERMES_ASSERT(!Text.Text.empty());

			std::vector<std::pair<uint32, Vec2>> GlyphPositions;
			UI::TextLayout::Layout(Text.Text, Text.FontSize, *Text.Font, [&](uint32 GlyphIndex, Vec2 GlyphPosition)
			{
				GlyphPositions.emplace_back(GlyphIndex, GlyphPosition);
			});
			
			FontPack.Repack();
			TextDescriptorSet->UpdateWithImageAndSampler(0, 0, FontPack.GetImage(), *TextFontSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			auto TextLocation = Vec2(Text.Rect.Min);
			for (auto [GlyphIndex, GlyphPosition] : GlyphPositions)
			{
				GlyphPosition += TextLocation;

				auto GlyphRectInPack = FontPack.GetGlyphCoordinates(*Text.Font, GlyphIndex, Text.FontSize);
				auto GlyphWidth = static_cast<float>(GlyphRectInPack.Width());
				auto GlyphHeight = static_cast<float>(GlyphRectInPack.Height());

				auto GlyphUVMin = Vec2(GlyphRectInPack.Min) / Vec2(FontPack.GetImage().GetDimensions());
				auto GlyphUVMax = Vec2(GlyphRectInPack.Max) / Vec2(FontPack.GetImage().GetDimensions());

				auto GlyphUVTopLeft = Vec2{ GlyphUVMin.X, GlyphUVMin.Y };
				auto GlyphUVBottomLeft = Vec2{ GlyphUVMin.X, GlyphUVMax.Y };
				auto GlyphUVTopRight = Vec2{ GlyphUVMax.X, GlyphUVMin.Y };
				auto GlyphUVBottomRight = Vec2{ GlyphUVMax.X, GlyphUVMax.Y };

				auto TopLeft = ((GlyphPosition) / Vec2(CurrentDimensions)) * 2.0f - 1.0f;
				auto TopRight = ((GlyphPosition + Vec2(GlyphWidth, 0)) / Vec2(CurrentDimensions)) * 2.0f - 1.0f;
				auto BottomLeft = ((GlyphPosition + Vec2(0, GlyphHeight)) / Vec2(CurrentDimensions)) * 2.0f - 1.0f;
				auto BottomRight = ((GlyphPosition + Vec2(GlyphWidth, GlyphHeight)) / Vec2(CurrentDimensions)) * 2.0f - 1.0f;

				TextVertices.emplace_back(TopLeft, GlyphUVTopLeft);
				TextVertices.emplace_back(TopRight, GlyphUVTopRight);
				TextVertices.emplace_back(BottomLeft, GlyphUVBottomLeft);
				TextVertices.emplace_back(BottomLeft, GlyphUVBottomLeft);
				TextVertices.emplace_back(TopRight, GlyphUVTopRight);
				TextVertices.emplace_back(BottomRight, GlyphUVBottomRight);
			}
		}

		TextMeshBuffer = Renderer::GetDevice().CreateBuffer(TextVertices.size() * sizeof(TextVertex), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		GPUInteractionUtilities::UploadDataToGPUBuffer(TextVertices.data(), TextVertices.size() * sizeof(TextVertex), 0, *TextMeshBuffer);

		HasTextToDraw = true;
	}
}
