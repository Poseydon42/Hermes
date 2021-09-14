#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
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
			VulkanDescriptorSetLayout(std::shared_ptr<VulkanDevice> InDevice, const std::vector<RenderInterface::DescriptorBinding>& Bindings);

			~VulkanDescriptorSetLayout() override;
			VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& Other);
			VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&& Other);

			VkDescriptorSetLayout GetDescriptorSetLayout() const { return Layout; }

		private:
			std::shared_ptr<VulkanDevice> Device;
			VkDescriptorSetLayout Layout;
		};
	}
}
