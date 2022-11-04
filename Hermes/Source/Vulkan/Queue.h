#pragma once

#include <memory>
#include <optional>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Vulkan/Device.h"
#include "Vulkan/Forward.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes::Vulkan
{
	/*
	 * A wrapper around VkQueue object and a VkCommandPool that was created for this particular queue
	 */
	class HERMES_API Queue
	{
		MAKE_NON_COPYABLE(Queue)
		MAKE_NON_MOVABLE(Queue)

	public:
		Queue(std::shared_ptr<Device::VkDeviceHolder> InDevice, uint32 InQueueFamilyIndex);

		~Queue();

		std::unique_ptr<CommandBuffer> CreateCommandBuffer(bool IsPrimaryBuffer = true) const;

		void SubmitCommandBuffer(const CommandBuffer& Buffer, std::optional<Fence*> Fence) const;

		void WaitForIdle() const;

		VkQueue GetQueue() const;

		uint32 GetQueueFamilyIndex() const;

	private:
		struct VkQueueHolder
		{
			MAKE_NON_COPYABLE(VkQueueHolder)
			MAKE_NON_MOVABLE(VkQueueHolder)
			ADD_DEFAULT_CONSTRUCTOR(VkQueueHolder)

			~VkQueueHolder();

			std::shared_ptr<Device::VkDeviceHolder> Device;
			VkQueue Queue = VK_NULL_HANDLE;
			VkCommandPool CommandPool = VK_NULL_HANDLE;
		};

		std::shared_ptr<VkQueueHolder> Holder;
		uint32 QueueFamilyIndex = 0;

		friend class CommandBuffer;
	};
}
