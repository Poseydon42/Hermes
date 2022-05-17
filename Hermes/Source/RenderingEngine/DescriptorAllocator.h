#pragma once

#include <array>
#include <memory>
#include <vector>

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"

namespace Hermes
{
	class HERMES_API DescriptorAllocator
	{
	public:
		DescriptorAllocator(std::shared_ptr<RenderInterface::Device> InDevice);

		std::shared_ptr<RenderInterface::DescriptorSet> Allocate(std::shared_ptr<RenderInterface::DescriptorSetLayout> Layout);

	private:
		std::shared_ptr<RenderInterface::Device> Device;
		std::vector<std::shared_ptr<RenderInterface::DescriptorSetPool>> PoolList;

		static constexpr uint32 DescriptorSetsPerPool = 1024;
		static constexpr std::array<RenderInterface::SubpoolDescription, 4> Subpools =
		{
			RenderInterface::SubpoolDescription{ RenderInterface::DescriptorType::UniformBuffer, 2 * DescriptorSetsPerPool },
			RenderInterface::SubpoolDescription{ RenderInterface::DescriptorType::SampledImage, 4 * DescriptorSetsPerPool },
			RenderInterface::SubpoolDescription{ RenderInterface::DescriptorType::CombinedSampler, 2 * DescriptorSetsPerPool },
			RenderInterface::SubpoolDescription{ RenderInterface::DescriptorType::Sampler, 2 * DescriptorSetsPerPool }
		};

		void AllocateNewPool();
	};
}
