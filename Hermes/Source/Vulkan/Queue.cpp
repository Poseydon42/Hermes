#include "Queue.h"

#include "Vulkan/Fence.h"
#include "Vulkan/CommandBuffer.h"

namespace Hermes::Vulkan
{
	Queue::Queue(std::shared_ptr<Device::VkDeviceHolder> InDevice, uint32 InQueueFamilyIndex)
		: Holder(std::make_unique<VkQueueHolder>())
		, QueueFamilyIndex(InQueueFamilyIndex)
	{
		Holder->Device = std::move(InDevice);

		vkGetDeviceQueue(Holder->Device->Device, QueueFamilyIndex, 0, &Holder->Queue);

		VkCommandPoolCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CreateInfo.queueFamilyIndex = QueueFamilyIndex;
		CreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECK_RESULT(vkCreateCommandPool(Holder->Device->Device, &CreateInfo, GVulkanAllocator, &Holder->CommandPool));
	}
		
	// Because VkQueue is automatically destroyed (released) when VkDevice is
	// destroyed, so we don't need to do anything ourselves
	Queue::~Queue()
	{
	}

	std::unique_ptr<CommandBuffer> Queue::CreateCommandBuffer(bool IsPrimaryBuffer/* = true*/) const
	{
		return std::make_unique<CommandBuffer>(Holder, IsPrimaryBuffer);
	}

	void Queue::SubmitCommandBuffer(const CommandBuffer& Buffer, std::optional<Fence*> Fence) const
	{;
		VkSubmitInfo SubmitInfo = {};
		SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SubmitInfo.commandBufferCount = 1;
		VkCommandBuffer TmpBufferCopy = Buffer.GetBuffer();
		SubmitInfo.pCommandBuffers = &TmpBufferCopy;
		VkFence VkFenceHandle = VK_NULL_HANDLE;
		if (Fence.has_value())
			VkFenceHandle = Fence.value()->GetFence();
		VK_CHECK_RESULT(vkQueueSubmit(Holder->Queue, 1, &SubmitInfo, VkFenceHandle));
	}

	void Queue::WaitForIdle() const
	{
		VK_CHECK_RESULT(vkQueueWaitIdle(Holder->Queue));
	}

	VkQueue Queue::GetQueue() const
	{
		return Holder->Queue;
	}

	uint32 Queue::GetQueueFamilyIndex() const
	{
		return QueueFamilyIndex;
	}

	Queue::VkQueueHolder::~VkQueueHolder()
	{
		vkDestroyCommandPool(Device->Device, CommandPool, GVulkanAllocator);
	}
}
