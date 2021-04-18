#pragma once

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Instance.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
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

			std::vector<RenderInterface::DeviceProperties> EnumerateAvailableDevices() override;

		private:
			void CreateDebugMessenger();
			
			VkInstance Instance;
			VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;
		};
	}
}
