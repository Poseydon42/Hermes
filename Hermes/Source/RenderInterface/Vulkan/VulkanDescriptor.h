#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
#include "RenderInterface/GenericRenderInterface/Buffer.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanDevice;

		class HERMES_API VulkanDescriptorSetLayout : public RenderInterface::DescriptorSetLayout
		{
			MAKE_NON_COPYABLE(VulkanDescriptorSetLayout)

		public:
			VulkanDescriptorSetLayout(std::shared_ptr<const VulkanDevice> InDevice, const std::vector<RenderInterface::DescriptorBinding>& Bindings);

			~VulkanDescriptorSetLayout() override;
			VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& Other);
			VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&& Other);

			VkDescriptorSetLayout GetDescriptorSetLayout() const { return Layout; }
			VkDescriptorType GetDescriptorType(uint32 BindingIndex) const { return DescriptorTypes[BindingIndex]; }

		private:
			std::shared_ptr<const VulkanDevice> Device;
			std::vector<VkDescriptorType> DescriptorTypes;
			VkDescriptorSetLayout Layout;
		};

		class HERMES_API VulkanDescriptorSetPool : public RenderInterface::DescriptorSetPool
		{
			MAKE_NON_COPYABLE(VulkanDescriptorSetPool)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(VulkanDescriptorSetPool)
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(VulkanDescriptorSetPool)

		public:
			VulkanDescriptorSetPool(std::shared_ptr<const VulkanDevice> InDevice, uint32 NumberOfSets, const std::vector<RenderInterface::SubpoolDescription>& Subpools, bool InSupportIndividualDeallocations);

			std::shared_ptr<RenderInterface::DescriptorSet> CreateDescriptorSet(std::shared_ptr<RenderInterface::DescriptorSetLayout> Layout) override;

			VkDescriptorPool GetDescriptorPool() const { return Holder->Pool; }

			uint32 GetNumberOfSets() const override { return NumSets; }

		private:
			// NOTE : we use this wrapper around VkDescriptorPool object to ensure that VkDescriptorPool gets destroyed
			//        only after all descriptors that it had allocated are destroyed. I used this hack instead of using
			//        shared_from_this because otherwise the end user would be forced to store VulkanDescriptorPool in
			//        a shared_ptr, which I do not want to enforce
			struct VkDescriptorPoolHolder
			{
				std::shared_ptr<const VulkanDevice> Device;
				VkDescriptorPool Pool;
			};
			friend class VulkanDescriptorSet;

			std::shared_ptr<VkDescriptorPoolHolder> Holder;
			uint32 NumSets;
			bool SupportIndividualDeallocations;
		};

		class HERMES_API VulkanDescriptorSet : public RenderInterface::DescriptorSet
		{
			MAKE_NON_COPYABLE(VulkanDescriptorSet)

		public:
			VulkanDescriptorSet(std::shared_ptr<const VulkanDevice> InDevice, std::shared_ptr<VulkanDescriptorSetPool::VkDescriptorPoolHolder> InPool, std::shared_ptr<VulkanDescriptorSetLayout> InLayout, VkDescriptorSet InSet, bool InFreeInDestructor);

			~VulkanDescriptorSet() override;
			VulkanDescriptorSet(VulkanDescriptorSet&& Other);
			VulkanDescriptorSet& operator=(VulkanDescriptorSet&& Other);

			virtual void UpdateWithBuffer(uint32 BindingIndex, uint32 ArrayIndex, const RenderInterface::Buffer& Buffer, uint32 Offset, uint32 Size) override;

			virtual void UpdateWithSampler(uint32 BindingIndex, uint32 ArrayIndex, const RenderInterface::Sampler& Sampler) override;

			virtual void UpdateWithImage(uint32 BindingIndex, uint32 ArrayIndex, const RenderInterface::Image& Image, RenderInterface::ImageLayout LayoutAtTimeOfAccess) override;

			virtual void UpdateWithImageAndSampler(uint32 BindingIndex, uint32 ArrayIndex, const RenderInterface::Image& Image, const RenderInterface::Sampler& Sampler, RenderInterface::ImageLayout LayoutAtTimeOfAccess) override;

			VkDescriptorSet GetDescriptorSet() const { return Set; }

		private:
			std::shared_ptr<const VulkanDevice> Device;
			std::shared_ptr<VulkanDescriptorSetPool::VkDescriptorPoolHolder> Pool;
			std::shared_ptr<VulkanDescriptorSetLayout> Layout;
			VkDescriptorSet Set;
			bool FreeInDestructor;
		};
	}
}
