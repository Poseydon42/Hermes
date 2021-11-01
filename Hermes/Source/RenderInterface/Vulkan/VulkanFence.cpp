#include "VulkanFence.h"

#include "VulkanDevice.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanFence::VulkanFence(std::shared_ptr<const VulkanDevice> InDevice, bool InitialState)
			: Device(std::move(InDevice))
			, Fence(VK_NULL_HANDLE)
		{
			VkFenceCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			if (InitialState)
				CreateInfo.flags |= VK_FENCE_CREATE_SIGNALED_BIT;
			VK_CHECK_RESULT(vkCreateFence(Device->GetDevice(), &CreateInfo, GVulkanAllocator, &Fence));
		}

		VulkanFence::~VulkanFence()
		{
			vkDestroyFence(Device->GetDevice(), Fence, GVulkanAllocator);
		}
		
		VulkanFence::VulkanFence(VulkanFence&& Other)
		{
			*this = std::move(Other);
		}

		VulkanFence& VulkanFence::operator=(VulkanFence&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(Fence, Other.Fence);
			return *this;
		}

		bool VulkanFence::IsSignaled() const
		{
			VkResult Result = vkGetFenceStatus(Device->GetDevice(), Fence);
			if (Result == VK_SUCCESS)
				return true;
			if (Result == VK_NOT_READY)
				return false;
			HERMES_ASSERT(false); // TODO : crash here?
			return false;
		}

		void VulkanFence::Wait(uint64 Timeout) const
		{
			vkWaitForFences(Device->GetDevice(), 1, &Fence, VK_TRUE, Timeout);
		}

		void VulkanFence::Reset()
		{
			vkResetFences(Device->GetDevice(), 1, &Fence);
		}
	}
}
