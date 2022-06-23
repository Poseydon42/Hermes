#include "VulkanDescriptor.h"

#include "RenderInterface/Vulkan/VulkanBuffer.h"
#include "RenderInterface/Vulkan/VulkanCommonTypes.h"
#include "RenderInterface/Vulkan/VulkanDevice.h"
#include "RenderInterface/Vulkan/VulkanImage.h"
#include "RenderInterface/Vulkan/VulkanSampler.h"

namespace Hermes
{
	namespace Vulkan
	{
		static VkDescriptorType DescriptorTypeToVkDescriptorType(RenderInterface::DescriptorType Type)
		{
			switch (Type)
			{
			case RenderInterface::DescriptorType::UniformBuffer:
				return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case RenderInterface::DescriptorType::Sampler:
				return VK_DESCRIPTOR_TYPE_SAMPLER;
			case RenderInterface::DescriptorType::SampledImage:
				return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			case RenderInterface::DescriptorType::CombinedSampler:
				return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case RenderInterface::DescriptorType::InputAttachment:
				return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			default:
				HERMES_ASSERT(false);
				return static_cast<VkDescriptorType>(0);
			}
		}

		VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(std::shared_ptr<const VulkanDevice> InDevice,
		                                                     const std::vector<RenderInterface::DescriptorBinding>&
		                                                     Bindings)
			: Device(std::move(InDevice))
			, Layout(VK_NULL_HANDLE)
		{
			std::vector<VkDescriptorSetLayoutBinding> VulkanBindings(Bindings.size());
			auto VulkanBinding = VulkanBindings.begin();
			DescriptorTypes.resize(Bindings.size());
			for (const auto& Binding : Bindings)
			{
				VulkanBinding->binding = Binding.Index;
				VulkanBinding->descriptorCount = Binding.DescriptorCount; // TODO
				VulkanBinding->descriptorType = DescriptorTypeToVkDescriptorType(Binding.Type);
				VulkanBinding->pImmutableSamplers = 0;
				VulkanBinding->stageFlags = ShaderTypeToVkShaderStage(Binding.Shader);

				DescriptorTypes[Binding.Index] = VulkanBinding->descriptorType;
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
			std::swap(DescriptorTypes, Other.DescriptorTypes);
			std::swap(Layout, Other.Layout);

			return *this;
		}

		VulkanDescriptorSetPool::VulkanDescriptorSetPool(std::shared_ptr<const VulkanDevice> InDevice,
		                                                 uint32 InNumberOfSets,
		                                                 const std::vector<RenderInterface::SubpoolDescription>&
		                                                 Subpools, bool InSupportIndividualDeallocations)
			: Holder(std::make_shared<VkDescriptorPoolHolder>(std::move(InDevice), VK_NULL_HANDLE))
			, NumSets(InNumberOfSets)
			, SupportIndividualDeallocations(InSupportIndividualDeallocations)
		{
			VkDescriptorPoolCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			CreateInfo.maxSets = InNumberOfSets;
			std::vector<VkDescriptorPoolSize> VulkanPoolSizes;
			for (const auto& Subpool : Subpools)
			{
				VkDescriptorPoolSize NewSize;
				NewSize.descriptorCount = Subpool.Count;
				NewSize.type = DescriptorTypeToVkDescriptorType(Subpool.Type);
				VulkanPoolSizes.push_back(NewSize);
			}
			CreateInfo.pPoolSizes = VulkanPoolSizes.data();
			CreateInfo.poolSizeCount = static_cast<uint32>(VulkanPoolSizes.size());
			CreateInfo.flags = SupportIndividualDeallocations ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0;

			VK_CHECK_RESULT(vkCreateDescriptorPool(Holder->Device->GetDevice(), &CreateInfo, GVulkanAllocator, &Holder->Pool))
		}

		std::shared_ptr<RenderInterface::DescriptorSet> VulkanDescriptorSetPool::CreateDescriptorSet(
			std::shared_ptr<RenderInterface::DescriptorSetLayout> Layout)
		{
			VkDescriptorSetAllocateInfo AllocateInfo = {};
			AllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			AllocateInfo.descriptorPool = Holder->Pool;
			AllocateInfo.descriptorSetCount = 1; // TODO : multi-set multi-layout constructor of some kind
			VkDescriptorSetLayout DescriptorLayout = static_cast<const VulkanDescriptorSetLayout&>(*Layout).
				GetDescriptorSetLayout();
			AllocateInfo.pSetLayouts = &DescriptorLayout;

			VkDescriptorSet AllocatedSet = VK_NULL_HANDLE;
			if (vkAllocateDescriptorSets(Holder->Device->GetDevice(), &AllocateInfo, &AllocatedSet) != VK_SUCCESS)
				return nullptr;
			return std::make_shared<VulkanDescriptorSet>(Holder->Device, Holder,
			                                             std::reinterpret_pointer_cast<
				                                             VulkanDescriptorSetLayout>(Layout), AllocatedSet,
			                                             SupportIndividualDeallocations);
		}

		VulkanDescriptorSetPool::VkDescriptorPoolHolder::VkDescriptorPoolHolder(
			std::shared_ptr<const VulkanDevice> InDevice,
			VkDescriptorPool InPool)
			: Device(std::move(InDevice))
			, Pool(InPool)
		{
		}

		VulkanDescriptorSetPool::VkDescriptorPoolHolder::~VkDescriptorPoolHolder()
		{
			vkDestroyDescriptorPool(Device->GetDevice(), Pool, GVulkanAllocator);
		}

		VulkanDescriptorSetPool::VkDescriptorPoolHolder::VkDescriptorPoolHolder(VkDescriptorPoolHolder&& Other)
		{
			*this = std::move(Other);
		}

		VulkanDescriptorSetPool::VkDescriptorPoolHolder& VulkanDescriptorSetPool::VkDescriptorPoolHolder::operator=(VkDescriptorPoolHolder&& Other)
		{
			std::swap(Device, Other.Device);
			std::swap(Pool, Other.Pool);

			return *this;
		}

		VulkanDescriptorSet::VulkanDescriptorSet(std::shared_ptr<const VulkanDevice> InDevice,
		                                         std::shared_ptr<VulkanDescriptorSetPool::VkDescriptorPoolHolder> InPool,
		                                         std::shared_ptr<VulkanDescriptorSetLayout> InLayout,
		                                         VkDescriptorSet InSet, bool InFreeInDescriptor)
			: Device(std::move(InDevice))
			, Pool(std::move(InPool))
			, Layout(std::move(InLayout))
			, Set(InSet)
			, FreeInDestructor(InFreeInDescriptor)
		{
		}

		VulkanDescriptorSet::~VulkanDescriptorSet()
		{
			if (FreeInDestructor)
				vkFreeDescriptorSets(Device->GetDevice(), Pool->Pool, 1, &Set);
			Set = VK_NULL_HANDLE;
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
			std::swap(FreeInDestructor, Other.FreeInDestructor);

			return *this;
		}

		void VulkanDescriptorSet::UpdateWithBuffer(uint32 BindingIndex, uint32 ArrayIndex,
		                                           const RenderInterface::Buffer& Buffer, uint32 Offset, uint32 Size)
		{
			VkWriteDescriptorSet Write = {};
			Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			Write.descriptorCount = 1;
			Write.dstSet = Set;
			Write.descriptorType = Layout->GetDescriptorType(BindingIndex);
			Write.dstBinding = BindingIndex;
			Write.dstArrayElement = ArrayIndex;
			VkDescriptorBufferInfo BufferInfo;
			BufferInfo.buffer = reinterpret_cast<const VulkanBuffer&>(Buffer).GetBuffer();
			BufferInfo.offset = Offset;
			BufferInfo.range = Size;
			Write.pBufferInfo = &BufferInfo;

			vkUpdateDescriptorSets(Device->GetDevice(), 1, &Write, 0, nullptr);
		}

		void VulkanDescriptorSet::UpdateWithSampler(uint32 BindingIndex, uint32 ArrayIndex,
		                                            const RenderInterface::Sampler& Sampler)
		{
			VkWriteDescriptorSet Write = {};
			Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			Write.descriptorCount = 1;
			Write.dstSet = Set;
			Write.descriptorType = Layout->GetDescriptorType(BindingIndex);
			Write.dstBinding = BindingIndex;
			Write.dstArrayElement = ArrayIndex;
			VkDescriptorImageInfo ImageInfo = {};
			ImageInfo.sampler = static_cast<const VulkanSampler&>(Sampler).GetSampler();
			Write.pImageInfo = &ImageInfo;

			vkUpdateDescriptorSets(Device->GetDevice(), 1, &Write, 0, nullptr);
		}

		void VulkanDescriptorSet::UpdateWithImage(uint32 BindingIndex, uint32 ArrayIndex,
		                                          const RenderInterface::ImageView& Image,
		                                          RenderInterface::ImageLayout LayoutAtTimeOfAccess)
		{
			VkWriteDescriptorSet Write = {};
			Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			Write.descriptorCount = 1;
			Write.dstSet = Set;
			Write.descriptorType = Layout->GetDescriptorType(BindingIndex);
			Write.dstBinding = BindingIndex;
			Write.dstArrayElement = ArrayIndex;
			VkDescriptorImageInfo ImageInfo = {};
			ImageInfo.imageView = static_cast<const VulkanImageView&>(Image).GetImageView();
			ImageInfo.imageLayout = ImageLayoutToVkImageLayout(LayoutAtTimeOfAccess);
			Write.pImageInfo = &ImageInfo;

			vkUpdateDescriptorSets(Device->GetDevice(), 1, &Write, 0, nullptr);
		}

		void VulkanDescriptorSet::UpdateWithImageAndSampler(uint32 BindingIndex, uint32 ArrayIndex,
		                                                    const RenderInterface::ImageView& Image,
		                                                    const RenderInterface::Sampler& Sampler,
		                                                    RenderInterface::ImageLayout LayoutAtTimeOfAccess)
		{
			VkWriteDescriptorSet Write = {};
			Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			Write.descriptorCount = 1;
			Write.dstSet = Set;
			Write.descriptorType = Layout->GetDescriptorType(BindingIndex);
			Write.dstBinding = BindingIndex;
			Write.dstArrayElement = ArrayIndex;
			VkDescriptorImageInfo ImageInfo = {};
			ImageInfo.imageView = static_cast<const VulkanImageView&>(Image).GetImageView();
			ImageInfo.imageLayout = ImageLayoutToVkImageLayout(LayoutAtTimeOfAccess);
			ImageInfo.sampler = static_cast<const VulkanSampler&>(Sampler).GetSampler();
			Write.pImageInfo = &ImageInfo;

			vkUpdateDescriptorSets(Device->GetDevice(), 1, &Write, 0, nullptr);
		}
	}
}
