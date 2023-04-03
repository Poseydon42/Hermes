#pragma once

#include "Core/Core.h"
#include "RenderingEngine/SharedData.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "UIEngine/Window.h"

namespace Hermes
{
	class HERMES_API UIPass
	{
	public:
		UIPass();

		const PassDesc& GetPassDescription() const;

		void AddWindow(const UI::Window* Window, Vec2ui ScreenLocation);

	private:
		std::vector<std::pair<const UI::Window*, Vec2ui>> Windows;
		
		std::unique_ptr<Vulkan::Pipeline> Pipeline;
		std::unique_ptr<Vulkan::DescriptorSetLayout> DescriptorSetLayout;
		std::unique_ptr<Vulkan::DescriptorSet> DescriptorSet;

		std::unique_ptr<Vulkan::Buffer> RectangleListBuffer;

		PassDesc Description;

		void PassCallback(const PassCallbackInfo& CallbackInfo);

		void CollectPrimitives(const UI::Window* Window, std::vector<RectanglePrimitive>& Rectangles);

		void CreatePipeline(const Vulkan::RenderPass& RenderPass, Vec2 ViewportDimensions);
	};
}
