#include "VulkanInstance.h"

#include <array>

#include "Core/Misc/StringUtils.h"

namespace Hermes
{
	namespace Vulkan
	{

// TODO : add command line argument instead of predefining it
#if defined(HERMES_DEBUG) || defined(HERMES_DEVELOPMENT)
#define HERMES_ENABLE_VK_VALIDATION
#endif

		static constexpr std::array<const char*, 3> RequiredExtensions =
		{
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#ifdef HERMES_PLATFORM_WINDOWS
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
		};

		static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
			VkDebugUtilsMessageTypeFlagsEXT,
			const VkDebugUtilsMessengerCallbackDataEXT* Data,
			void*)
		{
			switch (Severity)
			{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				HERMES_LOG_WARNING(L"Vulkan validation layer: %s", StringUtils::ANSIToString(ANSIString(Data->pMessage)).c_str());
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				HERMES_LOG_ERROR(L"Vulkan validation layer: %s", StringUtils::ANSIToString(ANSIString(Data->pMessage)).c_str());
				break;
			default:
				break;
			}

			return VK_FALSE;
		}

		VulkanInstance::VulkanInstance()
		{
			VkApplicationInfo AppInfo = {};
			AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			AppInfo.apiVersion = VK_API_VERSION_1_0;
			AppInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
			AppInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
			AppInfo.pApplicationName = "Game"; // TODO : set actual app name
			AppInfo.pEngineName = "Hermes Engine";

			bool ValidationLayersEnabled = false;
			const char* ValidationLayerName = "VK_LAYER_KHRONOS_validation";
#ifdef HERMES_ENABLE_VK_VALIDATION
			uint32_t LayersCount;
			std::vector<VkLayerProperties> AvailableLayers;
			vkEnumerateInstanceLayerProperties(&LayersCount, NULL);
			AvailableLayers.resize(LayersCount);
			vkEnumerateInstanceLayerProperties(&LayersCount, AvailableLayers.data());
			for (auto& Layer : AvailableLayers)
			{
				if (strcmp(Layer.layerName, ValidationLayerName) == 0)
				{
					ValidationLayersEnabled = true;
				}
			}
			if (!ValidationLayersEnabled)
			{
				HERMES_LOG_WARNING(L"Vulkan instance does not support validation layers. Vulkan will be loaded without them");
			}
#endif
			
			VkInstanceCreateInfo CreateInfo = {};
			CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			CreateInfo.enabledExtensionCount = (uint32)RequiredExtensions.size();
			CreateInfo.ppEnabledExtensionNames = RequiredExtensions.data();
			if (ValidationLayersEnabled)
			{
				CreateInfo.enabledLayerCount = 1;
				CreateInfo.ppEnabledLayerNames = &ValidationLayerName;
			}
			CreateInfo.pApplicationInfo = &AppInfo;

			VK_CHECK_RESULT(vkCreateInstance(&CreateInfo, GVulkanAllocator, &Instance));

			if (Instance == VK_NULL_HANDLE)
			{
				HERMES_ASSERT_LOG(false, L"Failed to create Vulkan instance");
			}

			if (ValidationLayersEnabled)
			{
				CreateDebugMessenger();
			}
		}

		VulkanInstance::~VulkanInstance()
		{
			if (DebugMessenger != VK_NULL_HANDLE)
			{
				auto* vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
				vkDestroyDebugUtilsMessenger(Instance, DebugMessenger, GVulkanAllocator);
			}
			vkDestroyInstance(Instance, GVulkanAllocator);
		}

		VulkanInstance::VulkanInstance(VulkanInstance&& Other)
		{
			std::swap(Instance, Other.Instance);
			std::swap(DebugMessenger, Other.DebugMessenger);
		}

		VulkanInstance& VulkanInstance::operator=(VulkanInstance&& Other)
		{
			std::swap(Instance, Other.Instance);
			std::swap(DebugMessenger, Other.DebugMessenger);
			return *this;
		}

		std::vector<RenderInterface::DeviceProperties> VulkanInstance::EnumerateAvailableDevices()
		{
			std::vector<RenderInterface::DeviceProperties> Result;
			
			std::vector<VkPhysicalDevice> AvailableDevices;
			uint32_t DeviceCount = 0;
			
			VK_CHECK_RESULT(vkEnumeratePhysicalDevices(Instance, &DeviceCount, 0))
			Result.resize(DeviceCount);
			AvailableDevices.resize(DeviceCount);
			VK_CHECK_RESULT(vkEnumeratePhysicalDevices(Instance, &DeviceCount, AvailableDevices.data()))

			for (size_t i = 0; i < AvailableDevices.size(); i++)
			{
				VkPhysicalDeviceProperties Properties;
				std::vector<VkQueueFamilyProperties> QueueFamilies;
				uint32_t QueueFamiliesCount = 0;
				
				vkGetPhysicalDeviceProperties(AvailableDevices[i], &Properties);
				Result[i].Name = Properties.deviceName;
				
				vkGetPhysicalDeviceQueueFamilyProperties(AvailableDevices[i], &QueueFamiliesCount, 0);
				QueueFamilies.resize(QueueFamiliesCount);
				vkGetPhysicalDeviceQueueFamilyProperties(AvailableDevices[i], &QueueFamiliesCount, QueueFamilies.data());
				
				Result[i].QueueFamilies.reserve(QueueFamiliesCount);
				for (auto& Family : QueueFamilies)
				{
					RenderInterface::QueueFamilyProperties QueueFamilyProps = {};
					QueueFamilyProps.Count = Family.queueCount;
					if (Family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
						QueueFamilyProps.Type |= RenderInterface::QueueFamilyType::Graphics;
					if (Family.queueFlags & VK_QUEUE_TRANSFER_BIT)
						QueueFamilyProps.Type |= RenderInterface::QueueFamilyType::Transfer;
					Result[i].QueueFamilies.push_back(QueueFamilyProps);
				}
			}

			return Result;
		}

		void VulkanInstance::CreateDebugMessenger()
		{
			VkDebugUtilsMessengerCreateInfoEXT DbgCreateInfo = {};
			DbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			DbgCreateInfo.pfnUserCallback = VulkanDebugCallback;
			DbgCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			DbgCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

			auto* vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");

			VK_CHECK_RESULT(vkCreateDebugUtilsMessenger(Instance, &DbgCreateInfo, GVulkanAllocator, &DebugMessenger))
		}
	}
}
