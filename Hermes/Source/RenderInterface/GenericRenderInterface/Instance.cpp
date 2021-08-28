#include "Instance.h"

#include "RenderInterface/Vulkan/VulkanInstance.h"

namespace Hermes
{
	namespace RenderInterface
	{
		std::shared_ptr<Instance> Instance::CreateRenderInterfaceInstance(const IPlatformWindow& Window)
		{
#ifdef HERMES_PLATFORM_WINDOWS
			return std::make_shared<Vulkan::VulkanInstance>(Window);
#endif
		}
	}
}
