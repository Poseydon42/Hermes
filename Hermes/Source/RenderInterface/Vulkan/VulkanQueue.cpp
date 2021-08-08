#include "VulkanQueue.h"

#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanQueue::VulkanQueue(std::shared_ptr<VulkanDevice> InDevice, uint32 InQueueFamilyIndex)
			: Device(InDevice)
			, QueueFamilyIndex(InQueueFamilyIndex)
		{
			vkGetDeviceQueue(Device->GetDevice(), QueueFamilyIndex, 0, &Queue);

			VkCommandPoolCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			CreateInfo.queueFamilyIndex = QueueFamilyIndex;
			VK_CHECK_RESULT(vkCreateCommandPool(Device->GetDevice(), &CreateInfo, GVulkanAllocator, &CommandPool));
		}

		VulkanQueue::~VulkanQueue()
		{
			vkDestroyCommandPool(Device->GetDevice(), CommandPool, GVulkanAllocator);
		}

		VulkanQueue::VulkanQueue(VulkanQueue&& Other)
		{
			*this = std::move(Other);
		}

		VulkanQueue& VulkanQueue::operator=(VulkanQueue&& Other)
		{
			std::swap(Queue, Other.Queue);
			std::swap(CommandPool, Other.CommandPool);
			std::swap(Device, Other.Device);
			std::swap(QueueFamilyIndex, Other.QueueFamilyIndex);
			return *this;
		}

		std::shared_ptr<RenderInterface::CommandBuffer> VulkanQueue::CreateCommandBuffer(bool IsPrimaryBuffer)
		{
			return std::make_shared<VulkanCommandBuffer>(Device, CommandPool, IsPrimaryBuffer);
		}
	}
}
