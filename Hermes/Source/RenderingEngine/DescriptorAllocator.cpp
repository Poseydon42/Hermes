#include "DescriptorAllocator.h"

#include "RenderingEngine/Renderer.h"
#include "Vulkan/Device.h"

namespace Hermes
{
	DescriptorAllocator::DescriptorAllocator()
	{
		AllocateNewPool();
	}

	std::unique_ptr<Vulkan::DescriptorSet> DescriptorAllocator::Allocate(const Vulkan::DescriptorSetLayout& Layout)
	{
		for (const auto& Pool : PoolList)
		{
			auto Result = Pool->AllocateDescriptorSet(Layout);
			if (Result != nullptr)
				return Result;
		}
		AllocateNewPool();
		auto Result = PoolList.back()->AllocateDescriptorSet(Layout);
		HERMES_ASSERT(Result);
		return Result;
	}

	void DescriptorAllocator::AllocateNewPool()
	{
		auto& Device = Renderer::GetDevice();
		PoolList.push_back(Device.CreateDescriptorSetPool(DescriptorSetsPerPool, { Subpools.begin(), Subpools.end() }));
	}
}
