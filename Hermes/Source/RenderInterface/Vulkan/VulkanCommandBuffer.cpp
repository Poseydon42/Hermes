#include "VulkanCommandBuffer.h"

#include "VulkanDevice.h"
#include "RenderInterface/Vulkan/VulkanBuffer.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanCommandBuffer::VulkanCommandBuffer(std::shared_ptr<VulkanDevice> InDevice, VkCommandPool InPool, bool IsPrimaryBuffer)
			: Device(InDevice)
			, Pool(InPool)
		{
			VkCommandBufferAllocateInfo AllocateInfo = {};
			AllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			AllocateInfo.commandPool = Pool;
			AllocateInfo.commandBufferCount = 1; // TODO : add a way to allocate multiple buffers at one time
			if (IsPrimaryBuffer)
				AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			else
				AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			VK_CHECK_RESULT(vkAllocateCommandBuffers(Device->GetDevice(), &AllocateInfo, &Buffer));
		}

		VulkanCommandBuffer::~VulkanCommandBuffer()
		{
			vkFreeCommandBuffers(Device->GetDevice(), Pool, 1, &Buffer);
		}

		VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& Other)
		{
			*this = std::move(Other);
		}

		VulkanCommandBuffer& VulkanCommandBuffer::operator=(VulkanCommandBuffer&& Other)
		{
			std::swap(Buffer, Other.Buffer);
			std::swap(Device, Other.Device);
			std::swap(Pool, Other.Pool);
			return *this;
		}

		void VulkanCommandBuffer::BeginRecording()
		{
			VkCommandBufferBeginInfo BeginInfo = {};
			BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			VK_CHECK_RESULT(vkBeginCommandBuffer(Buffer, &BeginInfo));
		}

		void VulkanCommandBuffer::EndRecording()
		{
			VK_CHECK_RESULT(vkEndCommandBuffer(Buffer));
		}

		void VulkanCommandBuffer::CopyBuffer(const std::shared_ptr<RenderInterface::Buffer>& Source,
		                                     const std::shared_ptr<RenderInterface::Buffer>& Destination,
		                                     std::vector<RenderInterface::BufferCopyRegion> CopyRegions)
		{
			const auto* VulkanSourceBuffer = static_cast<VulkanBuffer*>(Source.get());
			const auto* VulkanDestinationBuffer = static_cast<VulkanBuffer*>(Destination.get());
			std::vector<VkBufferCopy> VulkanCopyRegions(CopyRegions.size());
			auto It = VulkanCopyRegions.begin();
			for (const auto& CopyRegion : CopyRegions)
			{
				auto& VulkanCopyRegion = *It;
				VulkanCopyRegion.srcOffset = CopyRegion.SourceOffset;
				VulkanCopyRegion.dstOffset = CopyRegion.DestinationOffset;
				VulkanCopyRegion.size = CopyRegion.NumBytes;
				++It;
			}
			vkCmdCopyBuffer(Buffer, VulkanSourceBuffer->GetAsBuffer(), VulkanDestinationBuffer->GetAsBuffer(), (uint32)VulkanCopyRegions.size(), VulkanCopyRegions.data());
		}
	}
}
