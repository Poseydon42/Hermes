#pragma once

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"
#include "Vulkan.h"

namespace Hermes
{
	class IPlatformWindow;
}

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanInstance;
		class VulkanQueue;
		
		class HERMES_API VulkanDevice : public RenderInterface::Device, public std::enable_shared_from_this<VulkanDevice>
		{
			MAKE_NON_COPYABLE(VulkanDevice)
		public:
			VulkanDevice(VkPhysicalDevice InPhysicalDevice, std::shared_ptr<const VulkanInstance> InInstance, VkSurfaceKHR InSurface, std::weak_ptr<const IPlatformWindow> InWindow);
			
			~VulkanDevice() override;
			VulkanDevice(VulkanDevice&& Other);
			VulkanDevice& operator=(VulkanDevice&& Other);

			std::shared_ptr<RenderInterface::Swapchain> CreateSwapchain(uint32 NumFrames) const override;

			const RenderInterface::Queue& GetQueue(RenderInterface::QueueType Type) const override;
			
			std::shared_ptr<RenderInterface::Buffer> CreateBuffer(size_t Size, RenderInterface::BufferUsageType Usage) const override;

			std::shared_ptr<RenderInterface::Image> CreateImage(
				Vec2ui Size, RenderInterface::ImageUsageType Usage, RenderInterface::DataFormat Format, 
				uint32 MipLevels, RenderInterface::ImageLayout InitialLayout) const override;

			std::shared_ptr<RenderInterface::Fence> CreateFence(bool InitialState) const override;
			
			std::shared_ptr<RenderInterface::Shader> CreateShader(
				const String& Path, RenderInterface::ShaderType Type) const override;
			
			std::shared_ptr<RenderInterface::RenderPass> CreateRenderPass(
				const RenderInterface::RenderPassDescription& Description) const override;

			std::shared_ptr<RenderInterface::Pipeline> CreatePipeline(
				std::shared_ptr<RenderInterface::RenderPass> RenderPass,
				const RenderInterface::PipelineDescription& Description) const override;

			std::shared_ptr<RenderInterface::RenderTarget> CreateRenderTarget(
				std::shared_ptr<RenderInterface::RenderPass> RenderPass, 
				const std::vector<std::shared_ptr<RenderInterface::Image>>& Attachments, Vec2ui Size) const override;

			std::shared_ptr<RenderInterface::DescriptorSetLayout> CreateDescriptorSetLayout(
				const std::vector<RenderInterface::DescriptorBinding>& Bindings) const override;

			std::shared_ptr<RenderInterface::DescriptorSetPool> CreateDescriptorSetPool(
				uint32 NumberOfSets, const std::vector<RenderInterface::SubpoolDescription>& Subpools) const override;

			std::shared_ptr<RenderInterface::Sampler> CreateSampler(
				const RenderInterface::SamplerDescription& Description) const override;
			
			void WaitForIdle() const override;

			VmaAllocator GetAllocator() const { return Allocator; }
			VkDevice GetDevice() const { return Device; }
			
		private:
			VkDevice Device;
			VkPhysicalDevice PhysicalDevice;
			std::shared_ptr<const VulkanInstance> Instance;
			VkSurfaceKHR Surface;
			VmaAllocator Allocator;
			std::weak_ptr<const IPlatformWindow> Window;

			int32 RenderQueueIndex = -1, TransferQueueIndex = -1;

			mutable std::shared_ptr<RenderInterface::Queue> RenderQueue;
			mutable std::shared_ptr<RenderInterface::Queue> TransferQueue;
			mutable std::shared_ptr<RenderInterface::Queue> PresentationQueue;
		};
	}
}
