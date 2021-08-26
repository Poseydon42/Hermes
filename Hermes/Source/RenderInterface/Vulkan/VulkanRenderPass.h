#pragma once

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/RenderPass.h"
#include "Vulkan.h"

#include <memory>

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanDevice;

		class VulkanRenderPass : public RenderInterface::RenderPass
		{
		public:
			VulkanRenderPass(std::shared_ptr<VulkanDevice> InDevice, const RenderInterface::RenderPassDescription& Description);
			~VulkanRenderPass() override;

		private:
			std::shared_ptr<VulkanDevice> Device;
			VkRenderPass RenderPass;
		};
	}
}
