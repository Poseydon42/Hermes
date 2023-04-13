#pragma once

#include "Core/Core.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "UIEngine/Widgets/Widget.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Descriptor.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes
{
	class HERMES_API UIPass
	{
	public:
		UIPass();

		const PassDesc& GetPassDescription() const;

		void SetDrawingContext(const UI::DrawingContext* NewDrawingContext);

	private:
		const UI::DrawingContext* DrawingContext = nullptr;
		
		std::unique_ptr<Vulkan::Pipeline> Pipeline;
		std::unique_ptr<Vulkan::DescriptorSetLayout> DescriptorSetLayout;
		std::unique_ptr<Vulkan::DescriptorSet> DescriptorSet;

		std::unique_ptr<Vulkan::Buffer> RectangleListBuffer;

		PassDesc Description;

		void PassCallback(const PassCallbackInfo& CallbackInfo);

		void CreatePipeline(VkFormat ColorAttachmentFormat);
	};
}
