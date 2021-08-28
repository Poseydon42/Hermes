#include "VulkanResource.h"

#include "VulkanDevice.h"

namespace Hermes
{
	namespace Vulkan
	{

		VulkanResource::VulkanResource(std::shared_ptr<VulkanDevice> InDevice, size_t BufferSize, RenderInterface::ResourceUsageType Usage)
			: ResourceBase(RenderInterface::ResourceType::Buffer, BufferSize)
			, As {}
			, Device(std::move(InDevice))
			, Allocation(VK_NULL_HANDLE)
			, Mapped(false)
		{
			VmaAllocator Allocator = Device->GetAllocator();
			VkBufferCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			CreateInfo.size = BufferSize;
			if ((bool)(Usage & RenderInterface::ResourceUsageType::VertexBuffer))
				CreateInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			if ((bool)(Usage & RenderInterface::ResourceUsageType::IndexBuffer))
				CreateInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			if ((bool)(Usage & RenderInterface::ResourceUsageType::UniformBuffer))
				CreateInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			if ((bool)(Usage & RenderInterface::ResourceUsageType::CopySource))
				CreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			if ((bool)(Usage & RenderInterface::ResourceUsageType::CopyDestination))
				CreateInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo AllocationInfo = {};
			if ((bool)(Usage & RenderInterface::ResourceUsageType::CPUAccessible))
				AllocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			else
				AllocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			VK_CHECK_RESULT(vmaCreateBuffer(Allocator, &CreateInfo, &AllocationInfo, &As.Buffer.Handle, &Allocation, nullptr));
		}

		VulkanResource::VulkanResource(VulkanResource&& Other)
		{
			*this = std::move(Other);
		}

		VulkanResource& VulkanResource::operator=(VulkanResource&& Other)
		{
			std::swap(Allocation, Other.Allocation);
			std::swap(As, Other.As);
			return *this;
		}

		VulkanResource::~VulkanResource()
		{
			VmaAllocator Allocator = Device->GetAllocator();
			if (Mapped)
				Unmap();
			switch (ResourceBase::GetResourceType())
			{
			case RenderInterface::ResourceType::Buffer:
				vmaDestroyBuffer(Allocator, As.Buffer.Handle, Allocation);
				break;
			case RenderInterface::ResourceType::Unknown:
			default:
				HERMES_LOG_FATAL(L"Unknow resource type.");
			}
		}

		void* VulkanResource::Map()
		{
			void* Result;
			VmaAllocator Allocator = Device->GetAllocator();
			VK_CHECK_RESULT(vmaMapMemory(Allocator, Allocation, &Result));
			Mapped = true;
			return Result;
		}

		void VulkanResource::Unmap()
		{
			VmaAllocator Allocator = Device->GetAllocator();
			vmaUnmapMemory(Allocator, Allocation);
			Mapped = false;
		}

		VkBuffer VulkanResource::GetAsBuffer() const
		{
			if (GetResourceType() == RenderInterface::ResourceType::Buffer)
				return As.Buffer.Handle;
			else
				return VK_NULL_HANDLE;
		}
	}
}
