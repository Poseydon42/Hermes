#pragma once

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Platform/GenericPlatform/PlatformWindow.h"
#include "Vulkan/Forward.h"
#include "Vulkan/Instance.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes::Vulkan
{
	/*
	 * Represents an interface to a logical GPU instance that lets you interact with its capabilities
	 * by creating other objects
	 */
	class HERMES_API Device
	{
		MAKE_NON_COPYABLE(Device)
		MAKE_NON_MOVABLE(Device)

	public:
		Device(std::shared_ptr<Instance::VkInstanceHolder> InInstance, VkPhysicalDevice InPhysicalDevice,
		       const IPlatformWindow& InWindow);

		~Device();

		std::unique_ptr<Swapchain> CreateSwapchain(uint32 NumFrames) const;

		const Queue& GetQueue(VkQueueFlags Type) const;

		std::unique_ptr<Buffer> CreateBuffer(size_t Size, VkBufferUsageFlags Usage, bool IsMappable = false) const;

		std::unique_ptr<Image> CreateImage(Vec2ui Dimensions, VkImageUsageFlags Usage, VkFormat Format,
		                                   uint32 MipLevels) const;

		std::unique_ptr<Image> CreateCubemap(Vec2ui Dimensions, VkImageUsageFlags Usage, VkFormat Format,
		                                     uint32 MipLevels) const;

		std::unique_ptr<Fence> CreateFence(bool InitialState = false) const;

		std::unique_ptr<Shader> CreateShader(const String& Path, VkShaderStageFlagBits Type) const;

		std::unique_ptr<RenderPass> CreateRenderPass(
			const std::vector<std::pair<VkAttachmentDescription, AttachmentType>>& Attachments) const;

		std::unique_ptr<Pipeline> CreatePipeline(const RenderPass& RenderPass,
		                                         const PipelineDescription& Description) const;

		std::unique_ptr<Framebuffer> CreateFramebuffer(const RenderPass& RenderPass,
		                                               const std::vector<const ImageView*>& Attachments,
		                                               Vec2ui Dimensions) const;

		std::unique_ptr<DescriptorSetLayout> CreateDescriptorSetLayout(
			const std::vector<VkDescriptorSetLayoutBinding>& Bindings) const;

		std::unique_ptr<DescriptorSetPool> CreateDescriptorSetPool(uint32 NumberOfSets,
		                                                           const std::vector<VkDescriptorPoolSize>& Subpools,
		                                                           bool SupportIndividualDeallocations = false) const;

		std::unique_ptr<Sampler> CreateSampler(const SamplerDescription& Description) const;

		void WaitForIdle() const;

		VmaAllocator GetAllocator() const { return Holder->Allocator; }
		VkDevice GetDevice() const { return Holder->Device; }

	private:
		struct VkDeviceHolder
		{
			MAKE_NON_COPYABLE(VkDeviceHolder);
			MAKE_NON_MOVABLE(VkDeviceHolder);
			ADD_DEFAULT_CONSTRUCTOR(VkDeviceHolder);

			~VkDeviceHolder();

			std::shared_ptr<Instance::VkInstanceHolder> Instance;

			VkDevice Device = VK_NULL_HANDLE;
			VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
			VmaAllocator Allocator = VK_NULL_HANDLE;

			VkQueue PresentationQueue = VK_NULL_HANDLE;
		};

		std::shared_ptr<VkDeviceHolder> Holder;
		const IPlatformWindow& Window;

		std::unique_ptr<Queue> GraphicsQueue;
		std::unique_ptr<Queue> TransferQueue;

		friend class Buffer;
		friend class CommandBuffer;
		friend class DescriptorSet;
		friend class DescriptorSetLayout;
		friend class DescriptorSetPool;
		friend class Fence;
		friend class Framebuffer;
		friend class Image;
		friend class ImageView;
		friend class Pipeline;
		friend class Queue;
		friend class RenderPass;
		friend class Sampler;
		friend class Shader;
		friend class Swapchain;
	};
}
