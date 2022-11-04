#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Vulkan/Device.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes::Vulkan
{
	/*
	 * A wrapper around VkFence, the simplest GPU-to-CPU synchronization primitive
	 */
	class HERMES_API Fence
	{
		MAKE_NON_COPYABLE(Fence)
		MAKE_NON_MOVABLE(Fence)

	public:
		Fence(std::shared_ptr<Device::VkDeviceHolder> InDevice, bool InitialState);

		~Fence();

		/*
		 * Returns true if the fence is in 'signaled' state
		 */
		bool IsSignaled() const;

		/*
		 * Blocks current thread for at most Timeout nanoseconds waiting until the fence is signaled
		 *
		 * Returns immediately if the thread was signaled at the time of the call
		 */
		void Wait(uint64 Timeout) const;

		/*
		 * Resets the state of the fence to 'unsignaled'
		 */
		void Reset();

		VkFence GetFence() const;

	private:
		std::shared_ptr<Device::VkDeviceHolder> Device;

		VkFence Handle = VK_NULL_HANDLE;
	};
}
