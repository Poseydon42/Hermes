#pragma once

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"
#include "Vulkan.h"

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
			VulkanDevice(VkPhysicalDevice InPhysicalDevice, std::shared_ptr<VulkanInstance> InInstance, VkSurfaceKHR InSurface);
			
			~VulkanDevice() override;
			VulkanDevice(VulkanDevice&& Other);
			VulkanDevice& operator=(VulkanDevice&& Other);

			std::shared_ptr<RenderInterface::Swapchain> CreateSwapchain(Vec2i Size, uint32 NumFrames) override;

			std::shared_ptr<RenderInterface::Queue> GetQueue(RenderInterface::QueueType Type) override;
			
			std::shared_ptr<RenderInterface::Buffer> CreateBuffer(size_t Size, RenderInterface::BufferUsageType Usage) override;

			std::shared_ptr<RenderInterface::Fence> CreateFence(bool InitialState) override;
			
			std::shared_ptr<RenderInterface::Shader> CreateShader(const String& Path, RenderInterface::ShaderType Type) override;
			
			std::shared_ptr<RenderInterface::RenderPass> CreateRenderPass(const RenderInterface::RenderPassDescription& Description) override;

			std::shared_ptr<RenderInterface::Pipeline> CreatePipeline(std::shared_ptr<RenderInterface::RenderPass> RenderPass, const RenderInterface::PipelineDescription& Description) override;

			std::shared_ptr<RenderInterface::RenderTarget> CreateRenderTarget(std::shared_ptr<RenderInterface::RenderPass> RenderPass, const std::vector<std::shared_ptr<RenderInterface::Image>>& Attachments, Vec2ui Size) override;

			std::shared_ptr<RenderInterface::DescriptorSetLayout> CreateDescriptorSetLayout(const std::vector<RenderInterface::DescriptorBinding>& Bindings) override;

			std::shared_ptr<RenderInterface::DescriptorSetPool> CreateDescriptorSetPool(uint32 Size) override;

			std::shared_ptr<RenderInterface::Image> CreateImage(Vec2ui Size, RenderInterface::ImageUsageType Usage, RenderInterface::DataFormat Format, uint32 MipLevels, RenderInterface::ImageLayout InitialLayout) override;
			
			void WaitForIdle() override;

			VmaAllocator GetAllocator() const { return Allocator; }
			VkDevice GetDevice() const { return Device; }
			
		private:
			VkDevice Device;
			VkPhysicalDevice PhysicalDevice;
			std::shared_ptr<VulkanInstance> Instance;
			VkSurfaceKHR Surface;
			VmaAllocator Allocator;

			int32 RenderQueueIndex = -1, TransferQueueIndex = -1;

			std::shared_ptr<RenderInterface::Queue> RenderQueue;
			std::shared_ptr<RenderInterface::Queue> TransferQueue;
			std::shared_ptr<RenderInterface::Queue> PresentationQueue;
		};
	}
}
