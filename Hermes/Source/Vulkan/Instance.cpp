#include "Instance.h"

#include <array>

#include "Core/Misc/StringUtils.h"
#include "Vulkan/Device.h"

namespace Hermes::Vulkan
{
	VkSurfaceKHR CreateSurface(VkInstance Instance, const IPlatformWindow& Window)
	{
		VkSurfaceKHR Result = VK_NULL_HANDLE;
#ifdef HERMES_PLATFORM_WINDOWS
		VkWin32SurfaceCreateInfoKHR CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		CreateInfo.hinstance = GetModuleHandleW(nullptr);
		CreateInfo.hwnd = static_cast<HWND>(Window.GetNativeHandle());
		// On Windows this should only be HWND as we are not going to use anything other than WinAPI
		VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(Instance, &CreateInfo, GVulkanAllocator, &Result))
#endif
		return Result;
	}

	// TODO : add command line argument instead of predefining it
#if defined(HERMES_DEBUG) || defined(HERMES_DEVELOPMENT)
#define HERMES_ENABLE_VK_VALIDATION
#endif

	static constexpr std::array RequiredExtensions =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#ifdef HERMES_PLATFORM_WINDOWS
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
	};

	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
	                                                          VkDebugUtilsMessageTypeFlagsEXT,
	                                                          const VkDebugUtilsMessengerCallbackDataEXT* Data,
	                                                          void*)
	{
		switch (Severity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			HERMES_LOG_WARNING(L"Vulkan validation layer: %s",
			                   StringUtils::ANSIToString(ANSIString(Data->pMessage)).c_str());
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			HERMES_LOG_ERROR(L"Vulkan validation layer: %s",
			                 StringUtils::ANSIToString(ANSIString(Data->pMessage)).c_str());
			break;
		default:
			break;
		}

		return VK_FALSE;
	}

	static DeviceProperties GetPhysicalDeviceProperties(VkPhysicalDevice PhysicalDevice)
	{
		DeviceProperties Result = {};

		VkPhysicalDeviceProperties Properties = {};
		vkGetPhysicalDeviceProperties(PhysicalDevice, &Properties);
		VkPhysicalDeviceFeatures Features = {};
		vkGetPhysicalDeviceFeatures(PhysicalDevice, &Features);

		Result.Name = Properties.deviceName;
		Result.AnisotropySupport = Features.samplerAnisotropy;
		Result.MaxAnisotropyLevel = Properties.limits.maxSamplerAnisotropy;

		return Result;
	}

	Instance::Instance(const IPlatformWindow& InWindow)
		: Window(InWindow)
	{
		Holder = std::make_shared<VkInstanceHolder>();

		VkApplicationInfo AppInfo = {};
		AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		AppInfo.apiVersion = GVulkanVersion;
		AppInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
		AppInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
		AppInfo.pApplicationName = HERMES_GAME_NAME;
		AppInfo.pEngineName = "Hermes Engine";

		bool ValidationLayersEnabled = false;
		const char* ValidationLayerName = "VK_LAYER_KHRONOS_validation";
#ifdef HERMES_ENABLE_VK_VALIDATION
		uint32 LayersCount;
		std::vector<VkLayerProperties> AvailableLayers;
		vkEnumerateInstanceLayerProperties(&LayersCount, nullptr);
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
		CreateInfo.enabledExtensionCount = static_cast<uint32>(RequiredExtensions.size());
		CreateInfo.ppEnabledExtensionNames = RequiredExtensions.data();
		if (ValidationLayersEnabled)
		{
			CreateInfo.enabledLayerCount = 1;
			CreateInfo.ppEnabledLayerNames = &ValidationLayerName;
		}
		CreateInfo.pApplicationInfo = &AppInfo;

		VK_CHECK_RESULT(vkCreateInstance(&CreateInfo, GVulkanAllocator, &Holder->Instance));

		uint32 DeviceCount = 0;
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(Holder->Instance, &DeviceCount, nullptr));
		AvailableDeviceProperties.resize(DeviceCount);
		AvailableDevices.resize(DeviceCount);
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(Holder->Instance, &DeviceCount, AvailableDevices.data()));

		for (size_t Index = 0; Index < AvailableDevices.size(); Index++)
		{
			AvailableDeviceProperties[Index] = GetPhysicalDeviceProperties(AvailableDevices[Index]);
		}

		if (ValidationLayersEnabled)
		{
			CreateDebugMessenger();
		}


		Holder->Surface = CreateSurface(Holder->Instance, Window);
	}

	const std::vector<DeviceProperties>& Instance::EnumerateAvailableDevices() const
	{
		return AvailableDeviceProperties;
	}

	std::unique_ptr<Device> Instance::CreateDevice(size_t DeviceIndex) const
	{
		return std::make_unique<Device>(Holder, AvailableDevices[DeviceIndex], Window);
	}

	void Instance::CreateDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT DbgCreateInfo = {};
		DbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		DbgCreateInfo.pfnUserCallback = VulkanDebugCallback;
		DbgCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		DbgCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		auto* vkCreateDebugUtilsMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(Holder->Instance, "vkCreateDebugUtilsMessengerEXT"));
		HERMES_ASSERT(vkCreateDebugUtilsMessenger);

		VK_CHECK_RESULT(vkCreateDebugUtilsMessenger(Holder->Instance, &DbgCreateInfo, GVulkanAllocator, &Holder->
			                DebugMessenger));
	}

	Instance::VkInstanceHolder::~VkInstanceHolder()
	{
		vkDestroySurfaceKHR(Instance, Surface, GVulkanAllocator);
		if (DebugMessenger != VK_NULL_HANDLE)
		{
			auto* vkDestroyDebugUtilsMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
				vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT"));
			HERMES_ASSERT(vkDestroyDebugUtilsMessenger);
			vkDestroyDebugUtilsMessenger(Instance, DebugMessenger, GVulkanAllocator);
		}
		vkDestroyInstance(Instance, GVulkanAllocator);
	}
}
