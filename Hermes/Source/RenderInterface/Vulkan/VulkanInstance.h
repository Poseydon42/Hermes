#pragma once

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Instance.h"
#include "Vulkan.h"

namespace Hermes
{
	namespace Vulkan
	{
		class HERMES_API VulkanInstance : public RenderInterface::Instance
		{
			MAKE_NON_COPYABLE(VulkanInstance)
		public:
			VulkanInstance();
			
			~VulkanInstance() override;

			VulkanInstance(VulkanInstance&& Other);
			VulkanInstance& operator=(VulkanInstance&& Other);

		private:
			VkInstance Instance;
		};
	}
}
