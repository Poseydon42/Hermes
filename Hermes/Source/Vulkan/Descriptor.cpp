#include "Descriptor.h"

#include "Vulkan/Buffer.h"
#include "Vulkan/Image.h"
#include "Vulkan/Sampler.h"

namespace Hermes::Vulkan
{
	DescriptorSetLayout::DescriptorSetLayout(std::shared_ptr<Device::VkDeviceHolder> InDevice,
	                                         const std::vector<VkDescriptorSetLayoutBinding>& Bindings)
	{
		Holder = std::make_shared<VkDescriptorSetLayoutHolder>();
		Holder->Device = std::move(InDevice);

		for (const auto& Binding : Bindings)
		{
			HERMES_ASSERT(!Holder->DescriptorTypes.contains(Binding.binding));
			Holder->DescriptorTypes[Binding.binding] = Binding.descriptorType;
		}

		VkDescriptorSetLayoutCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		CreateInfo.bindingCount = static_cast<uint32>(Bindings.size());
		CreateInfo.pBindings = Bindings.data();

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(Holder->Device->Device, &CreateInfo, GVulkanAllocator, &Holder->
			                Layout));
	}

	VkDescriptorSetLayout DescriptorSetLayout::GetDescriptorSetLayout() const
	{
		return Holder->Layout;
	}

	VkDescriptorType DescriptorSetLayout::GetDescriptorType(uint32 BindingIndex) const
	{
		return Holder->DescriptorTypes.at(BindingIndex);
	}

	DescriptorSetLayout::VkDescriptorSetLayoutHolder::~VkDescriptorSetLayoutHolder()
	{
		vkDestroyDescriptorSetLayout(Device->Device, Layout, GVulkanAllocator);
	}

	DescriptorSetPool::DescriptorSetPool(std::shared_ptr<Device::VkDeviceHolder> InDevice, uint32 InNumberOfSets,
	                                     const std::vector<VkDescriptorPoolSize>& InPoolSizes,
	                                     bool InSupportIndividualDeallocations)
		: Holder(std::make_shared<VkDescriptorPoolHolder>())
		, NumSets(InNumberOfSets)
		, SupportIndividualDeallocations(InSupportIndividualDeallocations)
	{
		Holder->Device = std::move(InDevice);

		VkDescriptorPoolCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		CreateInfo.maxSets = InNumberOfSets;
		CreateInfo.pPoolSizes = InPoolSizes.data();
		CreateInfo.poolSizeCount = static_cast<uint32>(InPoolSizes.size());
		CreateInfo.flags = SupportIndividualDeallocations ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0;

		VK_CHECK_RESULT(vkCreateDescriptorPool(Holder->Device->Device, &CreateInfo, GVulkanAllocator, &Holder->Pool ));
	}

	std::unique_ptr<DescriptorSet> DescriptorSetPool::AllocateDescriptorSet(const DescriptorSetLayout& Layout)
	{
		auto VkLayout = Layout.GetDescriptorSetLayout();

		VkDescriptorSetAllocateInfo AllocateInfo = {};
		AllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		AllocateInfo.descriptorPool = Holder->Pool;
		AllocateInfo.descriptorSetCount = 1;
		AllocateInfo.pSetLayouts = &VkLayout;

		VkDescriptorSet AllocatedSet = VK_NULL_HANDLE;
		if (vkAllocateDescriptorSets(Holder->Device->Device, &AllocateInfo, &AllocatedSet) != VK_SUCCESS)
			return nullptr;
		return std::make_unique<DescriptorSet>(Layout.Holder, Holder, AllocatedSet, SupportIndividualDeallocations);
	}

	VkDescriptorPool DescriptorSetPool::GetDescriptorPool() const
	{
		return Holder->Pool;
	}

	uint32 DescriptorSetPool::GetNumberOfSets() const
	{
		return NumSets;
	}

	DescriptorSetPool::VkDescriptorPoolHolder::~VkDescriptorPoolHolder()
	{
		vkDestroyDescriptorPool(Device->Device, Pool, GVulkanAllocator);
	}

	DescriptorSet::DescriptorSet(std::shared_ptr<DescriptorSetLayout::VkDescriptorSetLayoutHolder> InLayout,
	                             std::shared_ptr<DescriptorSetPool::VkDescriptorPoolHolder> InPool,
	                             VkDescriptorSet InHandle, bool InFreeInDestructor)
		: Layout(std::move(InLayout))
		, Pool(std::move(InPool))
		, Handle(InHandle)
		, FreeInDestructor(InFreeInDestructor)
	{
	}

	DescriptorSet::~DescriptorSet()
	{
		if (FreeInDestructor)
			vkFreeDescriptorSets(Pool->Device->Device, Pool->Pool, 1, &Handle);
	}

	void DescriptorSet::UpdateWithBuffer(uint32 BindingIndex, uint32 ArrayIndex,
	                                     const Buffer& Buffer, uint32 Offset, uint32 Size)
	{
		VkWriteDescriptorSet Write = {};
		Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		Write.descriptorCount = 1;
		Write.dstSet = Handle;
		Write.descriptorType = Layout->DescriptorTypes.at(BindingIndex);
		Write.dstBinding = BindingIndex;
		Write.dstArrayElement = ArrayIndex;
		VkDescriptorBufferInfo BufferInfo;
		BufferInfo.buffer = Buffer.GetBuffer();
		BufferInfo.offset = Offset;
		BufferInfo.range = Size;
		Write.pBufferInfo = &BufferInfo;

		vkUpdateDescriptorSets(Pool->Device->Device, 1, &Write, 0, nullptr);
	}

	void DescriptorSet::UpdateWithSampler(uint32 BindingIndex, uint32 ArrayIndex,
	                                      const Sampler& Sampler)
	{
		VkWriteDescriptorSet Write = {};
		Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		Write.descriptorCount = 1;
		Write.dstSet = Handle;
		Write.descriptorType = Layout->DescriptorTypes.at(BindingIndex);
		Write.dstBinding = BindingIndex;
		Write.dstArrayElement = ArrayIndex;
		VkDescriptorImageInfo ImageInfo = {};
		ImageInfo.sampler = Sampler.GetSampler();
		Write.pImageInfo = &ImageInfo;

		vkUpdateDescriptorSets(Pool->Device->Device, 1, &Write, 0, nullptr);
	}

	void DescriptorSet::UpdateWithImage(uint32 BindingIndex, uint32 ArrayIndex, const ImageView& Image,
	                                    VkImageLayout LayoutAtTimeOfAccess)
	{
		VkWriteDescriptorSet Write = {};
		Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		Write.descriptorCount = 1;
		Write.dstSet = Handle;
		Write.descriptorType = Layout->DescriptorTypes.at(BindingIndex);
		Write.dstBinding = BindingIndex;
		Write.dstArrayElement = ArrayIndex;
		VkDescriptorImageInfo ImageInfo = {};
		ImageInfo.imageView = Image.GetImageView();
		ImageInfo.imageLayout = LayoutAtTimeOfAccess;
		Write.pImageInfo = &ImageInfo;

		vkUpdateDescriptorSets(Pool->Device->Device, 1, &Write, 0, nullptr);
	}

	void DescriptorSet::UpdateWithImageAndSampler(uint32 BindingIndex, uint32 ArrayIndex, const ImageView& Image,
	                                              const Sampler& Sampler, VkImageLayout LayoutAtTimeOfAccess)
	{
		VkWriteDescriptorSet Write = {};
		Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		Write.descriptorCount = 1;
		Write.dstSet = Handle;
		Write.descriptorType = Layout->DescriptorTypes.at(BindingIndex);
		Write.dstBinding = BindingIndex;
		Write.dstArrayElement = ArrayIndex;
		VkDescriptorImageInfo ImageInfo = {};
		ImageInfo.imageView = Image.GetImageView();
		ImageInfo.imageLayout = LayoutAtTimeOfAccess;
		ImageInfo.sampler = Sampler.GetSampler();
		Write.pImageInfo = &ImageInfo;

		vkUpdateDescriptorSets(Pool->Device->Device, 1, &Write, 0, nullptr);
	}

	VkDescriptorSet DescriptorSet::GetDescriptorSet() const
	{
		return Handle;
	}
}
