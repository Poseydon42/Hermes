#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Sampler.h"
#include "RenderInterface/Vulkan/Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanDevice;

		class HERMES_API VulkanSampler : public RenderInterface::Sampler
		{
			MAKE_NON_COPYABLE(VulkanSampler)
		public:
			VulkanSampler(std::shared_ptr<const VulkanDevice> InDevice, const RenderInterface::SamplerDescription& Description);

			~VulkanSampler() override;

			VulkanSampler(VulkanSampler&& Other);
			VulkanSampler& operator=(VulkanSampler&& Other);

			VkSampler GetSampler() const;

		private:
			std::shared_ptr<const VulkanDevice> Device;
			VkSampler Sampler;
		};
	}
}
