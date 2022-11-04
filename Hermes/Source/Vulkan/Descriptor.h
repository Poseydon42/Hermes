#pragma once

#include <memory>
#include <unordered_map>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Vulkan/Device.h"
#include "Vulkan/Forward.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes::Vulkan
{
	/*
	 * A wrapper around VkDescriptorSetLayout that also stores a map of binding types
	 */
	class HERMES_API DescriptorSetLayout
	{
		MAKE_NON_COPYABLE(DescriptorSetLayout)
		MAKE_NON_MOVABLE(DescriptorSetLayout)
		ADD_DEFAULT_DESTRUCTOR(DescriptorSetLayout)

	public:
		DescriptorSetLayout(std::shared_ptr<Device::VkDeviceHolder> InDevice,
		                    const std::vector<VkDescriptorSetLayoutBinding>& Bindings);

		VkDescriptorSetLayout GetDescriptorSetLayout() const;

		VkDescriptorType GetDescriptorType(uint32 BindingIndex) const;

	private:
		struct VkDescriptorSetLayoutHolder
		{
			MAKE_NON_COPYABLE(VkDescriptorSetLayoutHolder)
			MAKE_NON_MOVABLE(VkDescriptorSetLayoutHolder)
			ADD_DEFAULT_CONSTRUCTOR(VkDescriptorSetLayoutHolder)

			~VkDescriptorSetLayoutHolder();

			std::shared_ptr<Device::VkDeviceHolder> Device;

			VkDescriptorSetLayout Layout = VK_NULL_HANDLE;
			std::unordered_map<uint32, VkDescriptorType> DescriptorTypes;
		};

		std::shared_ptr<VkDescriptorSetLayoutHolder> Holder;

		friend class DescriptorSet;
		friend class DescriptorSetPool;
	};

	/*
	 * Wrapper around VkDescriptorSetPool
	 */
	class HERMES_API DescriptorSetPool
	{
		MAKE_NON_COPYABLE(DescriptorSetPool)
		MAKE_NON_MOVABLE(DescriptorSetPool)
		ADD_DEFAULT_DESTRUCTOR(DescriptorSetPool)

	public:
		DescriptorSetPool(std::shared_ptr<Device::VkDeviceHolder> InDevice, uint32 InNumberOfSets,
		                  const std::vector<VkDescriptorPoolSize>& InPoolSizes, bool InSupportIndividualDeallocations);

		/*
		 * Tries to allocate a new descriptor set with given layout from this pool
		 *
		 * If successful, returns an allocated descriptor set, otherwise returns null pointer
		 */
		std::unique_ptr<DescriptorSet> AllocateDescriptorSet(const DescriptorSetLayout& Layout);

		VkDescriptorPool GetDescriptorPool() const;

		uint32 GetNumberOfSets() const;

	private:
		// NOTE : we use this wrapper around VkDescriptorPool object to ensure that VkDescriptorPool gets destroyed
		//        only after all descriptors that it had allocated are destroyed. I used this hack instead of using
		//        shared_from_this because otherwise the end user would be forced to store VulkanDescriptorPool in
		//        a shared_ptr, which I do not want to enforce
		struct VkDescriptorPoolHolder
		{
			MAKE_NON_COPYABLE(VkDescriptorPoolHolder)
			MAKE_NON_MOVABLE(VkDescriptorPoolHolder)
			ADD_DEFAULT_CONSTRUCTOR(VkDescriptorPoolHolder)

			~VkDescriptorPoolHolder();

			std::shared_ptr<Device::VkDeviceHolder> Device;
			VkDescriptorPool Pool = VK_NULL_HANDLE;
		};

		std::shared_ptr<VkDescriptorPoolHolder> Holder;
		uint32 NumSets = 0;
		bool SupportIndividualDeallocations = false;

		friend class DescriptorSet;
	};

	/*
	 * A safe wrapper around VkDescriptorSet
	 *
	 * Holds a shader ownership of its layout and the pool it was allocated from, making sure
	 * that they are destroyed in the correct order and that VkDescriptorSetLayout is kept alive
	 * since it is required for UpdateWith* operations
	 */
	class HERMES_API DescriptorSet
	{
		MAKE_NON_COPYABLE(DescriptorSet)
		MAKE_NON_MOVABLE(DescriptorSet)

	public:
		DescriptorSet(std::shared_ptr<DescriptorSetLayout::VkDescriptorSetLayoutHolder> InLayout,
		              std::shared_ptr<DescriptorSetPool::VkDescriptorPoolHolder> InPool,
		              VkDescriptorSet InHandle, bool InFreeInDestructor);

		~DescriptorSet();

		/*
		 * Updates the descriptor set with a buffer
		 *
		 * Allowed for VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER and
		 * their *_DYNAMIC counterparts
		 */
		void UpdateWithBuffer(uint32 BindingIndex, uint32 ArrayIndex, const Buffer& Buffer, uint32 Offset,
		                      uint32 Size);

		/*
		 * Updates the descriptor set with a sampler only
		 *
		 * Allowed for VK_DESCRIPTOR_TYPE_SAMPLER
		 */
		void UpdateWithSampler(uint32 BindingIndex, uint32 ArrayIndex, const Sampler& Sampler);

		/*
		 * Updates the descriptor set with an image only
		 *
		 * Allowed for VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
		 * and VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
		 */
		void UpdateWithImage(uint32 BindingIndex, uint32 ArrayIndex, const ImageView& Image,
		                     VkImageLayout LayoutAtTimeOfAccess);

		/*
		 * Updates the descriptor with an image and a sampler at the same time
		 *
		 * Allowed for VK_DESCRIPTOR_TYPE_COMBINED_SAMPLER
		 */
		void UpdateWithImageAndSampler(uint32 BindingIndex, uint32 ArrayIndex, const ImageView& Image,
		                               const Sampler& Sampler, VkImageLayout LayoutAtTimeOfAccess);

		VkDescriptorSet GetDescriptorSet() const;

	private:
		std::shared_ptr<DescriptorSetLayout::VkDescriptorSetLayoutHolder> Layout;
		std::shared_ptr<DescriptorSetPool::VkDescriptorPoolHolder> Pool;

		VkDescriptorSet Handle = VK_NULL_HANDLE;
		bool FreeInDestructor = false;
	};
}
