#pragma once

#include "Core/Core.h"
#include "RenderingEngine/FontPack.h"
#include "RenderingEngine/SharedData.h"
#include "UIEngine/Widgets/Widget.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Descriptor.h"
#include "Vulkan/Image.h"
#include "Vulkan/Pipeline.h"
#include "vulkan/Sampler.h"

namespace Hermes
{
	class HERMES_API UIRenderer
	{
	public:
		UIRenderer();

		/**
		 * Processes the UI tree and prepares resources that will later be needed to actually draw the UI.
		 *
		 * @return Viewport rectangle where the scene will be rendered to
		 */
		Rect2Dui PrepareToRender(UI::Widget& RootWidget, Vec2ui RequiredDimensions);

		std::pair<const Vulkan::Image*, VkImageLayout> Render(const Vulkan::Image& RenderedScene, VkImageLayout RenderedSceneLayout);

	private:
		std::unique_ptr<Vulkan::Pipeline> RectanglePipeline;
		std::unique_ptr<Vulkan::DescriptorSet> RectangleDescriptorSet;
		static constexpr size_t MaxRectangleCount = 1024;
		std::unique_ptr<Vulkan::Buffer> RectangleListBuffer;

		static constexpr VkFormat TextFontImageFormat = VK_FORMAT_R8_UNORM;
		bool HasTextToDraw = false;
		std::unique_ptr<Vulkan::Pipeline> TextPipeline;
		std::unique_ptr<Vulkan::DescriptorSet> TextDescriptorSet;
		std::unique_ptr<Vulkan::Buffer> TextMeshBuffer;
		FontPack FontPack;
		std::unique_ptr<Vulkan::Sampler> TextFontSampler;

		static constexpr VkFormat DestinationImageFormat = VK_FORMAT_B8G8R8A8_SRGB;
		std::unique_ptr<Vulkan::Image> DestinationImage;
		std::unique_ptr<Vulkan::ImageView> DestinationImageView;

		Vec2ui CurrentDimensions = {};
		Rect2Dui SceneViewport = {};
		UIShaderPushConstants RectanglePushConstants = {};

		void RecreateDestinationImage();

		void PrepareToRenderRectangles(const UI::DrawingContext& DrawingContext);
		void PrepareToRenderText(const UI::DrawingContext& DrawingContext);
	};
}
