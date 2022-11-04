#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/FrameGraph/Graph.h"
#include "RenderingEngine/Passes/ForwardPass.h"
#include "RenderingEngine/Passes/PostProcessingPass.h"
#include "RenderingEngine/Passes/SkyboxPass.h"
#include "Vulkan/Device.h"
#include "Vulkan/Swapchain.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes
{
	class DescriptorAllocator;
	class Scene;
	class IPlatformWindow;

	struct GraphicsSettings
	{
		float AnisotropyLevel = 1.0;
	};

	class HERMES_API Renderer
	{
		MAKE_NON_COPYABLE(Renderer)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(Renderer)
		ADD_DEFAULT_DESTRUCTOR(Renderer)
		ADD_DEFAULT_CONSTRUCTOR(Renderer)

	public:
		static Renderer& Get();

		bool Init();

		const GraphicsSettings& GetGraphicsSettings() const;

		void UpdateGraphicsSettings(GraphicsSettings NewSettings);

		void RunFrame(const Scene& Scene);

		Vulkan::Device& GetActiveDevice();

		Vulkan::Swapchain& GetSwapchain();

		DescriptorAllocator& GetDescriptorAllocator();

		const Vulkan::DescriptorSetLayout& GetGlobalDataDescriptorSetLayout() const;

		const Vulkan::RenderPass& GetGraphicsRenderPassObject() const;

		const Vulkan::Sampler& GetDefaultSampler() const;

	private:
		Vulkan::DeviceProperties GPUProperties;
		GraphicsSettings CurrentSettings;

		std::unique_ptr<Vulkan::Instance> VulkanInstance;
		std::unique_ptr<Vulkan::Device> Device;
		std::unique_ptr<Vulkan::Swapchain> Swapchain;

		std::unique_ptr<DescriptorAllocator> DescriptorAllocator;
		std::unique_ptr<Vulkan::DescriptorSetLayout> GlobalDataDescriptorSetLayout;
		std::unique_ptr<Vulkan::Sampler> DefaultSampler;
		
		std::unique_ptr<FrameGraph> FrameGraph;
		std::unique_ptr<ForwardPass> ForwardPass;
		std::unique_ptr<PostProcessingPass> PostProcessingPass;
		std::unique_ptr<SkyboxPass> SkyboxPass;

		static constexpr uint32 NumberOfBackBuffers = 3; // TODO : let user modify
		static constexpr VkFormat ColorAttachmentFormat = VK_FORMAT_B8G8R8A8_UNORM;
		static constexpr VkFormat DepthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

		void DumpGPUProperties() const;
	};
}
