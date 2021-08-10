#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Fence.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class VulkanDevice;
		
		class HERMES_API VulkanFence final : public RenderInterface::Fence
		{
		public:
			MAKE_NON_COPYABLE(VulkanFence);

			VulkanFence(VulkanFence&& Other);
			VulkanFence& operator=(VulkanFence&& Other);
			
			~VulkanFence() override;
			
			VulkanFence(std::shared_ptr<VulkanDevice> InDevice, bool InitialState);

			bool IsSignaled() const override;
			
			void Wait(uint64 Timeout) const override;
			
			void Reset() override;

			VkFence GetFence() const { return Fence; }

		private:
			std::shared_ptr<VulkanDevice> Device;
			VkFence Fence;
		};
	}
}
