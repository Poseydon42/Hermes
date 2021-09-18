#include "VulkanBuffer.h"

#include "RenderInterface/Vulkan/VulkanDevice.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> InDevice, size_t BufferSize, RenderInterface::BufferUsageType Usage)
			: Buffer(VK_NULL_HANDLE)
			, Allocation(VK_NULL_HANDLE)
			, Device(std::move(InDevice))
			, IsMapped(false)
			, Size(BufferSize)
		{
			VmaAllocator Allocator = Device->GetAllocator();
			VkBufferCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			CreateInfo.size = BufferSize;
			if ((bool)(Usage & RenderInterface::BufferUsageType::VertexBuffer))
				CreateInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			if ((bool)(Usage & RenderInterface::BufferUsageType::IndexBuffer))
				CreateInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			if ((bool)(Usage & RenderInterface::BufferUsageType::UniformBuffer))
				CreateInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			if ((bool)(Usage & RenderInterface::BufferUsageType::CopySource))
				CreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			if ((bool)(Usage & RenderInterface::BufferUsageType::CopyDestination))
				CreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo AllocationInfo = {};
			if ((bool)(Usage & RenderInterface::BufferUsageType::CPUAccessible))
				AllocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			else
				AllocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			VK_CHECK_RESULT(vmaCreateBuffer(Allocator, &CreateInfo, &AllocationInfo, &Buffer, &Allocation, nullptr));
		}

		VulkanBuffer::VulkanBuffer(VulkanBuffer&& Other)
		{
			*this = std::move(Other);
		}

		VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& Other)
		{
			std::swap(Allocation, Other.Allocation);
			std::swap(Device, Other.Device);
			std::swap(IsMapped, Other.IsMapped);
			std::swap(Buffer, Other.Buffer);
			std::swap(Size, Other.Size);
			return *this;
		}

		VulkanBuffer::~VulkanBuffer()
		{
			VmaAllocator Allocator = Device->GetAllocator();
			if (IsMapped)
				Unmap();
			vmaDestroyBuffer(Allocator, Buffer, Allocation);
		}

		void* VulkanBuffer::Map()
		{
			void* Result;
			VmaAllocator Allocator = Device->GetAllocator();
			VK_CHECK_RESULT(vmaMapMemory(Allocator, Allocation, &Result));
			IsMapped = true;
			return Result;
		}

		void VulkanBuffer::Unmap()
		{
			VmaAllocator Allocator = Device->GetAllocator();
			vmaUnmapMemory(Allocator, Allocation);
			IsMapped = false;
		}

		size_t VulkanBuffer::GetSize() const
		{
			return Size;
		}

		VkBuffer VulkanBuffer::GetBuffer() const
		{
			return Buffer;
		}
	}
}
