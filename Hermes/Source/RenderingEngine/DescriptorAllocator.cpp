﻿#include "DescriptorAllocator.h"

#include "RenderInterface/GenericRenderInterface/Device.h"

namespace Hermes
{
	DescriptorAllocator::DescriptorAllocator(std::shared_ptr<RenderInterface::Device> InDevice)
		: Device(std::move(InDevice))
	{
		AllocateNewPool();
	}

	std::shared_ptr<RenderInterface::DescriptorSet> DescriptorAllocator::Allocate(std::shared_ptr<RenderInterface::DescriptorSetLayout> Layout)
	{
		for (const auto& Pool : PoolList)
		{
			auto Result = Pool->CreateDescriptorSet(Layout);
			if (Result != nullptr)
				return Result;
		}
		AllocateNewPool();
		auto Result = PoolList.back()->CreateDescriptorSet(Layout);
		HERMES_ASSERT(Result);
		return Result;
	}

	void DescriptorAllocator::AllocateNewPool()
	{
		auto NewPool = Device->CreateDescriptorSetPool(DescriptorSetsPerPool, { Subpools.begin(), Subpools.end() });
		PoolList.push_back(std::move(NewPool));
	}
}