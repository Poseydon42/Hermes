#include "VulkanInstance.h"

#include <array>

namespace Hermes
{
	namespace Vulkan
	{
		static constexpr std::array<const char*, 3> RequiredExtensions =
		{
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#ifdef HERMES_PLATFORM_WINDOWS
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
		};

		static constexpr std::array<const char*, 1> RequiredLayers =
		{
#if defined(HERMES_DEBUG) || defined(HERMES_DEVELOPMENT)
			"VK_LAYER_KHRONOS_VALIDATION"
#endif
		};

		VulkanInstance::VulkanInstance()
		{
			VkApplicationInfo AppInfo = {};
			AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			AppInfo.apiVersion = VK_API_VERSION_1_0;
			AppInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
			AppInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
			AppInfo.pApplicationName = "Game"; // TODO : set actual app name
			AppInfo.pEngineName = "Hermes Engine";

			VkInstanceCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			CreateInfo.enabledExtensionCount = RequiredExtensions.size();
			CreateInfo.ppEnabledExtensionNames = RequiredExtensions.data();
			CreateInfo.enabledLayerCount = RequiredLayers.size();
			CreateInfo.ppEnabledLayerNames = RequiredLayers.data();
			CreateInfo.pApplicationInfo = &AppInfo;

			VK_CHECK_RESULT(vkCreateInstance(&CreateInfo, GVulkanAllocator, &Instance))
		}

		VulkanInstance::~VulkanInstance()
		{
			vkDestroyInstance(Instance, GVulkanAllocator);
		}

		VulkanInstance::VulkanInstance(VulkanInstance&& Other)
		{
			std::swap(Instance, Other.Instance);
		}

		VulkanInstance& VulkanInstance::operator=(VulkanInstance&& Other)
		{
			std::swap(Instance, Other.Instance);
			return *this;
		}
	}
}
