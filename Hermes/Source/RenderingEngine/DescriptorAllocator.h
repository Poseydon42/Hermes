#pragma once

#include <array>
#include <memory>
#include <vector>

#include "Core/Core.h"
#include "Vulkan/Descriptor.h"
#include "Vulkan/Forward.h"

namespace Hermes
{
	class HERMES_API DescriptorAllocator
	{
		MAKE_NON_COPYABLE(DescriptorAllocator)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(DescriptorAllocator)
		ADD_DEFAULT_DESTRUCTOR(DescriptorAllocator)

	public:
		DescriptorAllocator();

		std::unique_ptr<Vulkan::DescriptorSet> Allocate(const Vulkan::DescriptorSetLayout& Layout);

	private:
		std::vector<std::unique_ptr<Vulkan::DescriptorSetPool>> PoolList;

		static constexpr uint32 DescriptorSetsPerPool = 1024;
		static constexpr std::array<VkDescriptorPoolSize, 5> Subpools =
		{
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 * DescriptorSetsPerPool },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1 * DescriptorSetsPerPool },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 * DescriptorSetsPerPool },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER, 1 * DescriptorSetsPerPool },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2 * DescriptorSetsPerPool }
		};

		void AllocateNewPool();
	};
}
