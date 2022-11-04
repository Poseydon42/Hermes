#include "Buffer.h"

#include "Vulkan/Device.h"

namespace Hermes::Vulkan
{
	Buffer::Buffer(std::shared_ptr<Device::VkDeviceHolder> InDevice, size_t BufferSize, VkBufferUsageFlags Usage,
	               bool IsMappable)
		: Device(std::move(InDevice))
		, Size(BufferSize)
	{
		VmaAllocator Allocator = Device->Allocator;
		VkBufferCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		CreateInfo.size = BufferSize;
		CreateInfo.usage = Usage;
		CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo AllocationInfo = {};
		AllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
		if (IsMappable)
			AllocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		else
			AllocationInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		VK_CHECK_RESULT(vmaCreateBuffer(Allocator, &CreateInfo, &AllocationInfo, &Handle, &Allocation, nullptr));
	}

	Buffer::~Buffer()
	{
		VmaAllocator Allocator = Device->Allocator;
		if (IsMapped)
			Unmap();
		vmaDestroyBuffer(Allocator, Handle, Allocation);
	}

	void* Buffer::Map()
	{
		void* Result;
		VmaAllocator Allocator = Device->Allocator;
		VK_CHECK_RESULT(vmaMapMemory(Allocator, Allocation, &Result));
		IsMapped = true;
		return Result;
	}

	void Buffer::Unmap()
	{
		VmaAllocator Allocator = Device->Allocator;
		vmaUnmapMemory(Allocator, Allocation);
		IsMapped = false;
	}

	size_t Buffer::GetSize() const
	{
		return Size;
	}

	VkBuffer Buffer::GetBuffer() const
	{
		return Handle;
	}
}
