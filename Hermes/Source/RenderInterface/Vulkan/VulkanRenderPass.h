﻿#pragma once

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

			MAKE_NON_COPYABLE(VulkanRenderPass)
			VulkanRenderPass(VulkanRenderPass&& Other);
			VulkanRenderPass& operator=(VulkanRenderPass&& Other);

			uint32 SubpassCount() const override { return SubpassNumber; }

			VkRenderPass GetRenderPass() const { return RenderPass; }
		
		private:
			std::shared_ptr<VulkanDevice> Device;
			uint32 SubpassNumber;
			VkRenderPass RenderPass;
		};
	}
}
