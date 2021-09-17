#include "VulkanDescriptor.h"

#include "RenderInterface/Vulkan/VulkanBuffer.h"
#include "RenderInterface/Vulkan/VulkanCommonTypes.h"
#include "RenderInterface/Vulkan/VulkanDevice.h"

namespace Hermes
{
	namespace Vulkan
	{
		VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(std::shared_ptr<VulkanDevice> InDevice, const std::vector<RenderInterface::DescriptorBinding>& Bindings)
			: Device(std::move(InDevice))
			, Layout(VK_NULL_HANDLE)
		{
			std::vector<VkDescriptorSetLayoutBinding> VulkanBindings(Bindings.size());
			auto VulkanBinding = VulkanBindings.begin();
			for (const auto& Binding : Bindings)
			{
				VulkanBinding->binding = Binding.Index;
				VulkanBinding->descriptorCount = Binding.DescriptorCount; // TODO
				VulkanBinding->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				VulkanBinding->pImmutableSamplers = 0;
				VulkanBinding->stageFlags = ShaderTypeToVkShaderStage(Binding.Shader);
				++VulkanBinding;
			}

			VkDescriptorSetLayoutCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			CreateInfo.bindingCount = static_cast<uint32>(VulkanBindings.size());
			CreateInfo.pBindings = VulkanBindings.data();

			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(Device->GetDevice(), &CreateInfo, GVulkanAllocator, &Layout));
		}

		VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
		{
			vkDestroyDescriptorSetLayout(Device->GetDevice(), Layout, GVulkanAllocator);
		}

		VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& Other)
		{
			*this = std::move(Other);
		}

		VulkanDescriptorSetLayout& VulkanDescriptorSetLayout::operator=(VulkanDescriptorSetLayout&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(Layout, Other.Layout);

			return *this;
		}

		VulkanDescriptorSetPool::VulkanDescriptorSetPool(std::shared_ptr<VulkanDevice> InDevice, uint32 Size)
			: Device(std::move(InDevice))
			, MaxSize(Size)
			, Pool(VK_NULL_HANDLE)
		{
			VkDescriptorPoolCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			CreateInfo.maxSets = Size;
			VkDescriptorPoolSize PoolSize = {};
			PoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // TODO : fix
			PoolSize.descriptorCount = Size;
			CreateInfo.pPoolSizes = &PoolSize;
			CreateInfo.poolSizeCount = 1;
			CreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // TODO : do we really need it?

			VK_CHECK_RESULT(vkCreateDescriptorPool(Device->GetDevice(), &CreateInfo, GVulkanAllocator, &Pool))
		}

		VulkanDescriptorSetPool::~VulkanDescriptorSetPool()
		{
			vkDestroyDescriptorPool(Device->GetDevice(), Pool, GVulkanAllocator);
		}

		VulkanDescriptorSetPool::VulkanDescriptorSetPool(VulkanDescriptorSetPool&& Other)
		{
			*this = std::move(Other);
		}

		VulkanDescriptorSetPool& VulkanDescriptorSetPool::operator=(VulkanDescriptorSetPool&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(MaxSize, Other.MaxSize);
			std::swap(Pool, Other.Pool);

			return *this;
		}

		std::shared_ptr<RenderInterface::DescriptorSet> VulkanDescriptorSetPool::CreateDescriptorSet(std::shared_ptr<RenderInterface::DescriptorSetLayout> Layout)
		{
			return std::make_shared<VulkanDescriptorSet>(Device, shared_from_this(), std::reinterpret_pointer_cast<VulkanDescriptorSetLayout>(Layout));
		}

		VulkanDescriptorSet::VulkanDescriptorSet(std::shared_ptr<VulkanDevice> InDevice, std::shared_ptr<VulkanDescriptorSetPool> InPool, std::shared_ptr<VulkanDescriptorSetLayout> InLayout)
			: Device(std::move(InDevice))
			, Pool(std::move(InPool))
			, Layout(std::move(InLayout))
			, Set(VK_NULL_HANDLE)
		{
			VkDescriptorSetAllocateInfo AllocateInfo = {};
			AllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			AllocateInfo.descriptorPool = Pool->GetDescriptorPool();
			AllocateInfo.descriptorSetCount = 1; // TODO : multi-set multi-layout constructor of some kind
			const VkDescriptorSetLayout DescriptorLayout = Layout->GetDescriptorSetLayout();
			AllocateInfo.pSetLayouts = &DescriptorLayout;

			VK_CHECK_RESULT(vkAllocateDescriptorSets(Device->GetDevice(), &AllocateInfo, &Set));
		}

		VulkanDescriptorSet::~VulkanDescriptorSet()
		{
			vkFreeDescriptorSets(Device->GetDevice(), Pool->GetDescriptorPool(), 1, &Set);
		}

		VulkanDescriptorSet::VulkanDescriptorSet(VulkanDescriptorSet&& Other)
		{
			*this = std::move(Other);
		}

		VulkanDescriptorSet& VulkanDescriptorSet::operator=(VulkanDescriptorSet&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(Pool, Other.Pool);
			std::swap(Layout, Other.Layout);
			std::swap(Set, Other.Set);

			return *this;
		}

		void VulkanDescriptorSet::Update(uint32 BindingIndex, uint32 ArrayIndex, const RenderInterface::Buffer& Buffer, uint32 Offset, uint32 Size)
		{
			VkWriteDescriptorSet Write = {};
			Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			Write.descriptorCount = 1;
			Write.dstSet = Set;
			Write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // TODO
			Write.dstBinding = BindingIndex;
			Write.dstArrayElement = ArrayIndex;
			VkDescriptorBufferInfo BufferInfo = {};
			BufferInfo.buffer = reinterpret_cast<const VulkanBuffer&>(Buffer).GetBuffer();
			BufferInfo.offset = Offset;
			BufferInfo.range = Size;
			Write.pBufferInfo = &BufferInfo;

			vkUpdateDescriptorSets(Device->GetDevice(), 1, &Write, 0, nullptr);
		}
	}
}
