#include "Fence.h"

namespace Hermes::Vulkan
{
	Fence::Fence(std::shared_ptr<Device::VkDeviceHolder> InDevice, bool InitialState)
		: Device(std::move(InDevice))
	{
		VkFenceCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		if (InitialState)
			CreateInfo.flags |= VK_FENCE_CREATE_SIGNALED_BIT;
		VK_CHECK_RESULT(vkCreateFence(Device->Device, &CreateInfo, GVulkanAllocator, &Handle));
	}

	Fence::~Fence()
	{
		vkDestroyFence(Device->Device, Handle, GVulkanAllocator);
	}

	bool Fence::IsSignaled() const
	{
		VkResult Result = vkGetFenceStatus(Device->Device, Handle);
		if (Result == VK_SUCCESS)
			return true;
		if (Result == VK_NOT_READY)
			return false;
		HERMES_ASSERT(false); // TODO : crash here?
		return false;
	}

	void Fence::Wait(uint64 Timeout) const
	{
		vkWaitForFences(Device->Device, 1, &Handle, true, Timeout);
	}

	void Fence::Reset()
	{
		vkResetFences(Device->Device, 1, &Handle);
	}

	VkFence Fence::GetFence() const
	{
		return Handle;
	}
}
