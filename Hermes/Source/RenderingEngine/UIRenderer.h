#pragma once

#include "Core/Core.h"
#include "RenderingEngine/FontPack.h"
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
		Rect2Dui PrepareToRender(const UI::Widget& RootWidget, Vec2ui RequiredDimensions);

		std::pair<const Vulkan::Image*, VkImageLayout> Render(const Vulkan::Image& RenderedScene, VkImageLayout RenderedSceneLayout);

	private:
		static constexpr uint32 RectanglePrimitivesDescriptorBinding = 0;
		static constexpr uint32 RectangleTexturesDescriptorBinding = 1;

		bool HasRectanglesToDraw = false;
		std::unique_ptr<Vulkan::Pipeline> RectanglePipeline;
		std::unique_ptr<Vulkan::DescriptorSet> RectangleDescriptorSet;
		std::unique_ptr<Vulkan::Sampler> RectangleTextureSampler;
		std::unique_ptr<Vulkan::Buffer> RectangleMeshBuffer;
		std::unique_ptr<Vulkan::Buffer> RectanglePrimitiveBuffer;
		std::vector<AssetHandle<Texture2D>> RectangleTextures; // NOTE: this is to ensure that the textures won't be destroyed during rendering
		AssetHandle<Texture2D> EmptyTexture; // NOTE: this is to bind to unused slots in the texture array descriptor to be fully compatible with the spec

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

		void RecreateDestinationImage();

		void PrepareToRenderRectangles(const UI::DrawingContext& DrawingContext);
		void PrepareToRenderText(const UI::DrawingContext& DrawingContext);
	};
}
