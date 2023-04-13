#pragma once

#include <memory>

#include "Core/Core.h"
#include "Math/Rect2D.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Scene/Scene.h"
#include "RenderingEngine/ShaderCache.h"
#include "UIEngine/Widgets/Widget.h"
#include "Vulkan/Forward.h"

namespace Hermes
{
	class HERMES_API Renderer
	{
	public:
		static bool Init();

		static void RunFrame(const Scene& Scene, const UI::Widget* RootWidget);

		static void Shutdown();

		static Vulkan::Device& GetDevice();

		static Vec2ui GetSwapchainDimensions();

		static DescriptorAllocator& GetDescriptorAllocator();

		static ShaderCache& GetShaderCache();

		static const Vulkan::DescriptorSetLayout& GetGlobalDataDescriptorSetLayout();

		static const Vulkan::Buffer& GetGlobalSceneDataBuffer();

		static const Vulkan::Sampler& GetDefaultSampler();

	private:
		static void DumpGPUProperties();

		static void FillSceneDataBuffer(const Scene& Scene);

		static void Present(const Vulkan::Image& SourceImage, VkImageLayout CurrentLayout, Rect2Dui Viewport);
	};
}
