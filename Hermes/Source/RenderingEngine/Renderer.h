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

		static void SetViewport(Rect2Dui NewViewport);

		static Vulkan::Device& GetDevice();

		static Vec2ui GetSwapchainDimensions();

		static DescriptorAllocator& GetDescriptorAllocator();

		static ShaderCache& GetShaderCache();

		static const Vulkan::DescriptorSetLayout& GetGlobalDataDescriptorSetLayout();

		static const Vulkan::RenderPass& GetGraphicsRenderPassObject();

		/*
		 * Returns a render pass object that will be compatible with any render pass that
		 * will utilize a 'vertex-only' pipeline of some material
		 */
		static const Vulkan::RenderPass& GetVertexRenderPassObject();

		static const Vulkan::Buffer& GetGlobalSceneDataBuffer();

		static const Vulkan::Sampler& GetDefaultSampler();

	private:
		static void DumpGPUProperties();

		static void FillSceneDataBuffer(const Scene& Scene);

		static void Present(const Vulkan::Image& SourceImage, VkImageLayout CurrentLayout, Rect2Dui Viewport);
	};
}
