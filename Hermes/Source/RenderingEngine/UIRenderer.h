#pragma once

#include "Core/Core.h"
#include "RenderingEngine/SharedData.h"
#include "UIEngine/Widgets/Widget.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Descriptor.h"
#include "Vulkan/Pipeline.h"

namespace Hermes
{
	class HERMES_API UIRenderer
	{
	public:
		UIRenderer();

		Rect2Dui PrepareToRender(const UI::Widget& RootWidget, Vec2ui RequiredDimensions);

		std::pair<const Vulkan::Image*, VkImageLayout> Render(const Vulkan::Image& RenderedScene, VkImageLayout RenderedSceneLayout);

	private:
		std::unique_ptr<Vulkan::Pipeline> Pipeline;
		std::unique_ptr<Vulkan::DescriptorSet> DescriptorSet;

		static constexpr VkFormat DestinationImageFormat = VK_FORMAT_B8G8R8A8_SRGB;
		std::unique_ptr<Vulkan::Image> DestinationImage;
		std::unique_ptr<Vulkan::ImageView> DestinationImageView;

		static constexpr size_t MaxRectangleCount = 1024;
		std::unique_ptr<Vulkan::Buffer> RectangleListBuffer;

		Vec2ui CurrentDimensions = {};
		UI::DrawingContext DrawingContext;
		UIShaderPushConstants PushConstants = {};

		void RecreateDestinationImage();
	};
}
