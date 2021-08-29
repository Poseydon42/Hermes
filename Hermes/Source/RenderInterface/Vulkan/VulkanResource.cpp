#include "VulkanResource.h"

#include "VulkanDevice.h"

namespace Hermes
{
	namespace Vulkan
	{

		VulkanResource::VulkanResource(std::shared_ptr<VulkanDevice> InDevice, size_t BufferSize, RenderInterface::ResourceUsageType Usage)
			: ResourceBase(RenderInterface::ResourceType::Buffer, BufferSize)
			, AsBuffer()
			, AsImage()
			, Allocation(VK_NULL_HANDLE)
			, Device(std::move(InDevice))
			, IsMapped(false)
			, IsOwned(true)
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
			VK_CHECK_RESULT(vmaCreateBuffer(Allocator, &CreateInfo, &AllocationInfo, &AsBuffer.Handle, &Allocation, nullptr));
		}

		VulkanResource::VulkanResource(std::shared_ptr<VulkanDevice> InDevice, VkImage Image, Vec2ui Size)
			: ResourceBase(RenderInterface::ResourceType::Image, 0) // Size is unknown because we don't own this image
			, AsBuffer()
			, AsImage()
			, Allocation(VK_NULL_HANDLE)
			, Device(std::move(InDevice))
			, IsMapped(false)
			, IsOwned(false)
		{
			AsImage.Size = Size;
			AsImage.Handle = Image;
		}

		VulkanResource::VulkanResource(VulkanResource&& Other)
		{
			*this = std::move(Other);
		}

		VulkanResource& VulkanResource::operator=(VulkanResource&& Other)
		{
			std::swap(Allocation, Other.Allocation);
			std::swap(AsBuffer, Other.AsBuffer);
			std::swap(AsImage, Other.AsImage);
			std::swap(Device, Other.Device);
			std::swap(IsMapped, Other.IsMapped);
			std::swap(IsOwned, Other.IsOwned);
			return *this;
		}

		VulkanResource::~VulkanResource()
		{
			if (!IsOwned)
				return;
			VmaAllocator Allocator = Device->GetAllocator();
			if (IsMapped)
				Unmap();
			switch (ResourceBase::GetResourceType())
			{
			case RenderInterface::ResourceType::Buffer:
				vmaDestroyBuffer(Allocator, AsBuffer.Handle, Allocation);
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
			IsMapped = true;
			return Result;
		}

		void VulkanResource::Unmap()
		{
			VmaAllocator Allocator = Device->GetAllocator();
			vmaUnmapMemory(Allocator, Allocation);
			IsMapped = false;
		}

		VkBuffer VulkanResource::GetAsBuffer() const
		{
			HERMES_ASSERT(GetResourceType() == RenderInterface::ResourceType::Buffer)
			return AsBuffer.Handle;
		}

		VkImage VulkanResource::GetAsImage() const
		{
			HERMES_ASSERT(GetResourceType() == RenderInterface::ResourceType::Image)
			return AsImage.Handle;
		}

		Vec2ui VulkanResource::GetImageSize() const
		{
			HERMES_ASSERT(GetResourceType() == RenderInterface::ResourceType::Image)
			return AsImage.Size;
		}
	}
}
