#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Vulkan/Device.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes::Vulkan
{
	/*
	 * A safe wrapper around VkBuffer and the memory allocation that is bound to it
	 */
	class HERMES_API Buffer
	{
		MAKE_NON_COPYABLE(Buffer)
		MAKE_NON_MOVABLE(Buffer)

	public:
		Buffer(std::shared_ptr<Device::VkDeviceHolder> InDevice, size_t BufferSize, VkBufferUsageFlags Usage,
		       bool IsMappable);

		~Buffer();

		void* Map();

		void Unmap();

		size_t GetSize() const;

		VkBuffer GetBuffer() const;

	private:
		std::shared_ptr<Device::VkDeviceHolder> Device;

		VkBuffer Handle = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;

		bool IsMapped = false;
		size_t Size = 0;
	};
}
