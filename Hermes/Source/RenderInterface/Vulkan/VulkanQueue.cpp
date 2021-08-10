#include "VulkanQueue.h"

#include "RenderInterface/Vulkan/VulkanCommandBuffer.h"
#include "RenderInterface/Vulkan/VulkanDevice.h"
#include "RenderInterface/Vulkan/VulkanFence.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanQueue::VulkanQueue(std::shared_ptr<VulkanDevice> InDevice, uint32 InQueueFamilyIndex)
			: Device(std::move(InDevice))
			, QueueFamilyIndex(InQueueFamilyIndex)
		{
			vkGetDeviceQueue(Device->GetDevice(), QueueFamilyIndex, 0, &Queue);

			VkCommandPoolCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			CreateInfo.queueFamilyIndex = QueueFamilyIndex;
			CreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
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

		void VulkanQueue::SubmitCommandBuffer(std::shared_ptr<RenderInterface::CommandBuffer> Buffer, std::optional<std::shared_ptr<RenderInterface::Fence>> Fence)
		{
			auto* VulkanBuffer = (VulkanCommandBuffer*)Buffer.get();
			VkSubmitInfo SubmitInfo = {};
			SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			SubmitInfo.commandBufferCount = 1;
			VkCommandBuffer TmpBufferCopy = VulkanBuffer->GetBuffer();
			SubmitInfo.pCommandBuffers = &TmpBufferCopy;
			VkFence VkFenceObject = VK_NULL_HANDLE;
			if (Fence.has_value())
			{
				auto* VulkanFenceObject = (VulkanFence*)Fence->get();
				VkFenceObject = VulkanFenceObject->GetFence();
			}
			VK_CHECK_RESULT(vkQueueSubmit(Queue, 1, &SubmitInfo, VkFenceObject));
		}
	}
}
